#include "Model.h"
#include "ModelBase.h"
#include "DirectXBase.h"
#include "kMath.h"
#include "TextureManager.h"
#include "Logger.h"
#include "SrvManager.h"
#include "SkyBox.h"
#include "json.hpp"

using namespace Logger;

void Model::Initialize(std::string directoryPath, std::string filename, bool enableLighting, bool isAnimation) {
	bool enableAnimationLoad = isAnimation;

	if (filename.ends_with(".obj"))
	{
		// モデル読み込み
		modelData = LoadModelFileOBJ(directoryPath, filename);
	}
	else if (filename.ends_with(".gltf"))
	{// モデル読み込み
		modelData = LoadModelFileGLTF(directoryPath, filename);
	}

	if (isAnimation)
	{
		this->isAnimation = enableAnimationLoad;
		Animation anim = LoadAnimationFile(directoryPath, filename);
		animation["DefaultAnimation"] = anim;
	}

	// 頂点Resourceの作成
	CreateVertexResource();

	// 頂点BufferResourceの作成
	CreateVertexBufferView();

	vertexData.resize(modelData.matVertexData.size());
	indexBufferView.resize(modelData.matVertexData.size());
	indexResource.resize(modelData.matVertexData.size());
	materialTemplateData.resize(modelData.materialTemplate.size());
	materialTemplateResource.resize(modelData.materialTemplate.size());
	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	for (uint32_t i = 0; i < modelData.matVertexData.size(); i++)
	{
		vertexResource.at(i)->Map(0, nullptr, reinterpret_cast<void**>(&vertexData[i]));
		std::memcpy(vertexData[i], modelData.matVertexData.at(i).vertices.data(), sizeof(VertexData) * modelData.matVertexData.at(i).vertices.size()); // 頂点データをリソースにコピー

		indexResource.at(i) = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(uint32_t) * modelData.matVertexData.at(i).indices.size());

		indexBufferView.at(i).BufferLocation = indexResource.at(i)->GetGPUVirtualAddress();
		indexBufferView.at(i).SizeInBytes = UINT(sizeof(uint32_t) * modelData.matVertexData.at(i).indices.size());
		indexBufferView.at(i).Format = DXGI_FORMAT_R32_UINT;

		indexResource.at(i)->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
		std::memcpy(mappedIndex, modelData.matVertexData.at(i).indices.data(), sizeof(uint32_t) * modelData.matVertexData.at(i).indices.size());

		materialTemplateResource.at(modelData.matVertexData[i].materialIndex) = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(MaterialTemplate) * modelData.materialTemplate.size());
		materialTemplateResource.at(modelData.matVertexData[i].materialIndex)->Map(0, nullptr, reinterpret_cast<void**>(&materialTemplateData[modelData.matVertexData[i].materialIndex]));
		std::memcpy(materialTemplateData[modelData.matVertexData[i].materialIndex], &modelData.materialTemplate.at(modelData.matVertexData[i].materialIndex), sizeof(MaterialTemplate) * modelData.materialTemplate.size());
	}

	CreateMaterialResouce();
	//  書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));


	// データを書き込む
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };

	materialData->uvTransform = MakeIdentity4x4();

	materialData->enableLighting = enableLighting;
	materialData->shininess = 70.0f;
	materialData->specularColor = { 1.0f, 1.0f, 1.0f };
	materialData->environmentCoefficient = 0.0f;
	materialData->enableMetallic = true;


	useWireFrameTexture = false;
	TextureManager::GetInstance()->LoadTexture("Resources/Debug/white1x1.png");
	whiteTextureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath("Resources/Debug/white1x1.png");

}

void Model::Draw() {

	// wvp用のCBufferの場所を設定
	ModelBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	
	for (uint32_t index = 0; index < modelData.matVertexData.size(); index++)
	{
		ModelBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(8, materialTemplateResource[modelData.matVertexData[index].materialIndex]->GetGPUVirtualAddress());
		if (isAnimation)
		{
			ModelBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootDescriptorTable(9, skinCluster[index].paletteSrvHandle.second);
			ModelBase::GetInstance()->GetDxBase()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView[0][index]); // VBVを設定
			ModelBase::GetInstance()->GetDxBase()->GetCommandList()->IASetVertexBuffers(1, 1, &vertexBufferView[1][index]); // VBVを設定
		}
		else
		{
			ModelBase::GetInstance()->GetDxBase()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView[0][index]); // VBVを設定
		}

		ModelBase::GetInstance()->GetDxBase()->GetCommandList()->IASetIndexBuffer(&indexBufferView.at(index)); // VBVを設定

		if (useWireFrameTexture)
		{
			SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, whiteTextureIndex);
		}
		else
		{
			SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, modelData.material[modelData.matVertexData[index].materialIndex].textureIndex);
		}
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(7, SkyBox::GetInstance()->GetSrvIndex());


		ModelBase::GetInstance()->GetDxBase()->GetCommandList()->DrawIndexedInstanced(UINT(modelData.matVertexData.at(index).indices.size()), 1, 0, 0, 0);
	}

}

void Model::SetSkinCluster(const std::vector<SkinCluster> skinCluster)
{
	this->skinCluster = skinCluster;
	vertexBufferView[1].resize(modelData.matVertexData.size());
	for (uint32_t i = 0; i < modelData.matVertexData.size(); i++)
	{
		vertexBufferView[1].at(i) = skinCluster[i].influenceBufferView;
	}

}

void Model::AddAnimation(std::string directoryPath, std::string filename, std::string animationName)
{
	Animation anim = LoadAnimationFile(directoryPath, filename);
	animation[animationName] = anim;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 1, 中で必要となる変数の宣言
	MaterialData materialData; // 構築するMaterialData
	std::string line;          // ファイルから読んだ１行を格納するもの
	// 2, ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());                             // とりあえず開けなかったら止める
	// 3, 実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
		else
		{
			// map_Kdが存在しなかったらwhite1x1をテクスチャとして使用する
			materialData.textureFilePath = "Resources/Debug/white1x1.png";
		}
	}
	// 4, MaterialDataを返す
	return materialData;
}

// マルチスレッド化予定
ModelData Model::LoadModelFileGLTF(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
	//assert(scene->HasMeshes()); // メッシュが無いのは対応しない
	if (!scene->HasMeshes())
	{
		Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
		modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
		return modelData;
	}
	// ↑パスが間違ってる可能性(大)

	// マルチマテリアル対応のためにメモリを保管しておく
	modelData.matVertexData.resize(scene->mNumMeshes);

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		//assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
		//assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応
		if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
		{
			Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
			modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
			return modelData;
		}

		modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
		modelData.matVertexData[meshIndex].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管
		modelData.matVertexData[meshIndex].materialIndex = mesh->mMaterialIndex;
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
		{
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			// 右手系->左手系への返還を忘れずに
			modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}
		// Indexの解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
		{
			aiFace& face = mesh->mFaces[faceIndex];
			//assert(face.mNumIndices == 3);
			if (face.mNumIndices != 3)
			{
				Log("指定したファイルに三角化されていない面が存在します\nフォルダをもう一度確認してください\n");
				modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
				return modelData;
			}

			for (uint32_t element = 0; element < face.mNumIndices; ++element)
			{
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);
				modelData.matVertexData[meshIndex].indices.push_back(vertexIndex);
			}
		}

		// SkinCluster構築用のデータ取得を追加
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
		{
			// Jointごとの格納領域を作る
			aiBone* bone = mesh->mBones[boneIndex];
			std::string JointName = bone->mName.C_Str();
			JointWeightData jointWeightData;// = modelData.skinClusterData[JointName];

			// InverseBindPoseMatrixの抽出
			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix({ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

			// Weight情報を取り出す
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
			{
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight , bone->mWeights[weightIndex].mVertexId });
			}
			modelData.skinClusterData[JointName] = jointWeightData;
			modelData.matVertexData[meshIndex].skinClusterData[JointName] = jointWeightData;
		}
	}
	// テクスチャが無い場合white1x1を張るようにする
	if (scene->mNumMaterials == 0)
	{
		MaterialData matData;
		matData.textureFilePath = "Resources/Debug/white1x1.png";

		// テクスチャ読み込み
		TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
		// 読み込んだテクスチャの番号尾を取得
		matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

		MaterialTemplate matTempData;
		// メタリックの数値
		matTempData.metallic = 0.0f;

		modelData.materialTemplate.push_back(matTempData);
		modelData.material.push_back(matData);
	}
	// マテリアルを設定されている数読み込む
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		// gltfは最後のマテリアルに何も入っていないのでcontinueする
		if (materialIndex == scene->mNumMaterials - 1)
		{
			continue;
		}

		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			MaterialData matData;
			matData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();

			// テクスチャ読み込み
			TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
			// 読み込んだテクスチャの番号尾を取得
			matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

			// メタリックの数値
			float metallic = 0.0f;
			MaterialTemplate matTempData;

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
				matTempData.metallic = metallic;
			}
			else
			{
				matTempData.metallic = 0.0f;
			}

			modelData.materialTemplate.push_back(matTempData);
			modelData.material.push_back(matData);


		}
		else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
		{
			MaterialData matData;
			matData.textureFilePath = "Resources/Debug/white1x1.png";

			// テクスチャ読み込み
			TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
			// 読み込んだテクスチャの番号尾を取得
			matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

			// メタリックの数値
			float metallic = 0.0f;
			MaterialTemplate matTempData;

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
				matTempData.metallic = metallic;
			}
			else
			{
				matTempData.metallic = 0.0f;
			}

			modelData.materialTemplate.push_back(matTempData);
			modelData.material.push_back(matData);

		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);
	return modelData;
}

Node Model::ReadNode(aiNode* node)
{
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する関数を利用

	result.transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
	result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // x軸を反転、さらに回転方向が逆なので軸を反転させる
	result.transform.translate = { -translate.x, translate.y, translate.z }; // x軸を反転
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);
	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
	{
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation result;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	if (scene->mNumAnimations == 0)// アニメーションが無い
	{
		Log("this scene have not Animation");
		assert(0);
	};
	aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初の差にメーションだけ採用。
	result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換
	// NodeAnimationを解析する
	// assimpでは個々のNodeのAnimationをchannelと読んでいるのでchannelを回してNodeAnimationの情報をとってくる
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex)
	{
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = result.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.translate.push_back(keyframe);
		}
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
		{
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w }; // 右手->左手
			nodeAnimation.rotate.push_back(keyframe);
		}
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.scale.push_back(keyframe);
		}
	}


	return result;
}

ModelData Model::LoadModelFileOBJ(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;            // 構築するModelData
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	//assert(scene->HasMeshes()); 
	// メッシュが無いのは対応しない
	if (!scene->HasMeshes())
	{
		Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
		modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
		return modelData;
	}

	modelData.matVertexData.resize(scene->mNumMeshes + 1);
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		//assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
		//assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応
		if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
		{
			Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
			modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
			return modelData;
		}

		modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
		modelData.matVertexData[meshIndex].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管
		if (mesh->mMaterialIndex != 0)
		{
			modelData.matVertexData[meshIndex].materialIndex = mesh->mMaterialIndex - 1;
		}
		else
		{
			modelData.matVertexData[meshIndex].materialIndex = mesh->mMaterialIndex;
		}
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
		{
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			// 右手系->左手系への返還を忘れずに
			modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.matVertexData[meshIndex].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}
		// Indexの解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
		{
			aiFace& face = mesh->mFaces[faceIndex];
			//assert(face.mNumIndices == 3);
			if (face.mNumIndices != 3)
			{
				Log("指定したファイルに三角化されていない面が存在します\nフォルダをもう一度確認してください\n");
				modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
				return modelData;
			}

			for (uint32_t element = 0; element < face.mNumIndices; element++)
			{
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);
				modelData.matVertexData[meshIndex].indices.push_back(vertexIndex);
			}
		}

	}
	// マテリアルが作成されていない場合はwhite1x1を使用
	if (scene->mNumMaterials == 1 || scene->mNumMaterials == 0)
	{
		MaterialData matData;
		matData.textureFilePath = "Resources/Debug/white1x1.png";

		// テクスチャ読み込み
		TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
		// 読み込んだテクスチャの番号尾を取得
		matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

		MaterialTemplate matTempData;
		// メタリックの数値		
		matTempData.metallic = 0.0f;

		modelData.materialTemplate.push_back(matTempData);
		modelData.material.push_back(matData);
	}
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		if (materialIndex == 0)
		{
			continue;
		}

		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			MaterialData matData;
			matData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();

			// テクスチャ読み込み
			TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
			// 読み込んだテクスチャの番号尾を取得
			matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

			// メタリックの数値
			float metallic = 0.0f;
			MaterialTemplate matTempData;

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
				matTempData.metallic = metallic;
			}
			else
			{
				matTempData.metallic = 0.0f;
			}

			modelData.materialTemplate.push_back(matTempData);
			modelData.material.push_back(matData);
		}
		else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
		{
			MaterialData matData;
			matData.textureFilePath = "Resources/Debug/white1x1.png";

			// テクスチャ読み込み
			TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);
			// 読み込んだテクスチャの番号尾を取得
			matData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(matData.textureFilePath);

			MaterialTemplate matTempData;
			// メタリックの数値		
			matTempData.metallic = 0.0f;

			modelData.materialTemplate.push_back(matTempData);
			modelData.material.push_back(matData);
		}
	}

	for (size_t i = 0; i < modelData.matVertexData.size(); i++)
	{
		if (modelData.matVertexData.at(i).vertices.empty())
		{
			modelData.matVertexData.erase(modelData.matVertexData.begin() + i);
		}
	}
	modelData.rootNode = ReadNode(scene->mRootNode);
	return modelData;
}

void Model::CreateVertexResource() {
	vertexResource.resize(modelData.matVertexData.size());
	for (uint32_t i = 0; i < modelData.matVertexData.size(); i++)
	{
		// 頂点リソースの作成
		vertexResource.at(i) = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(VertexData) * modelData.matVertexData.at(i).vertices.size());
	}
}

void Model::CreateVertexBufferView() {
	vertexBufferView[0].resize(modelData.matVertexData.size());
	for (uint32_t i = 0; i < modelData.matVertexData.size(); i++)
	{
		// 頂点バッファビューを作成する
		D3D12_VERTEX_BUFFER_VIEW vBv;
		vBv.BufferLocation = vertexResource.at(i)->GetGPUVirtualAddress();
		vBv.SizeInBytes = UINT(sizeof(VertexData) * modelData.matVertexData.at(i).vertices.size());
		vBv.StrideInBytes = sizeof(VertexData);

		vertexBufferView[0][i] = vBv;
	}
}

void Model::CreateMaterialResouce() {
	materialResource = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(Material));
}