#include "Model.h"
#include "ModelBase.h"
#include "DirectXBase.h"
#include "kMath.h"
#include "TextureManager.h"
#include "Logger.h"
#include "SrvManager.h"
#include "SkyBox.h"
#include "json.hpp"
#include "Object3dBase.h"
#include "SrvManager.h"
#include <thread>
#include <algorithm>

using namespace Logger;

void Model::Initialize(std::string directoryPath, std::string fileName, bool isAnimation) {
    bool enableAnimationLoad = isAnimation;

    if (fileName.ends_with(".obj"))
    {
        // モデル読み込み
        modelData = LoadModelFileOBJ(directoryPath, fileName);
    }
    else if (fileName.ends_with(".gltf"))
    {// モデル読み込み
        modelData = LoadModelFileGLTF(directoryPath, fileName);
    }

    if (isAnimation)
    {
        this->isAnimation = enableAnimationLoad;
        Animation anim = LoadAnimationFile(directoryPath, fileName);
        animation["DefaultAnimation"] = anim;
    }
    CreateAABB();
    // 頂点Resourceの作成
    CreateVertexResource();

    // 頂点BufferResourceの作成
    CreateVertexBufferView();

    vertexData.resize(modelData.matVertexData.size());
    indexBufferView.resize(modelData.matVertexData.size());
    indexResource.resize(modelData.matVertexData.size());
    materialTemplateData.resize(modelData.materialTemplate.size());
    materialTemplateResource.resize(modelData.materialTemplate.size());
    if (this->isAnimation)
    {
        // GPUskinning用リソースはメッシュ単位で保持する。
        paletteResource.resize(modelData.matVertexData.size());
        inputVertexResource.resize(modelData.matVertexData.size());
        influenceResource.resize(modelData.matVertexData.size());
        outputVertexResource.resize(modelData.matVertexData.size());
        skinningInformationResource.resize(modelData.matVertexData.size());
    }
    // VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
    int num = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        vertexResource.at(num)->Map(0, nullptr, reinterpret_cast<void**>(&vertexData[num]));
        std::memcpy(vertexData[num], matData.second.vertices.data(), sizeof(VertexData) * matData.second.vertices.size()); // 頂点データをリソースにコピー

        indexResource.at(num) = DirectXBase::GetInstance()->CreateBufferResource(sizeof(uint32_t) * matData.second.indices.size());

        indexBufferView.at(num).BufferLocation = indexResource.at(num)->GetGPUVirtualAddress();
        indexBufferView.at(num).SizeInBytes = UINT(sizeof(uint32_t) * matData.second.indices.size());
        indexBufferView.at(num).Format = DXGI_FORMAT_R32_UINT;

        indexResource.at(num)->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
        std::memcpy(mappedIndex, matData.second.indices.data(), sizeof(uint32_t) * matData.second.indices.size());

        materialTemplateResource.at(matData.second.materialIndex) = DirectXBase::GetInstance()->CreateBufferResource(sizeof(MaterialTemplate));
        materialTemplateResource.at(matData.second.materialIndex)->Map(0, nullptr, reinterpret_cast<void**>(&materialTemplateData[matData.second.materialIndex]));
        std::memcpy(materialTemplateData[matData.second.materialIndex], &modelData.materialTemplate.at(matData.second.materialIndex), sizeof(MaterialTemplate));
        num++;
    }
}

void Model::SkinningUpdate(const Skeleton& skeleton) {
    if (!isAnimation)
    {
        return;
    }

    size_t meshIndex = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        if (meshIndex >= mappedPalette.size() || meshIndex >= skinCluster.size())
        {
            ++meshIndex;
            continue;
        }

        const size_t jointCount = std::min(skeleton.joints.size(), skinCluster[meshIndex].inverseBindPoseMatrices.size());
        for (size_t jointIndex = 0; jointIndex < jointCount; ++jointIndex)
        {
            mappedPalette[meshIndex][jointIndex].skeletonSpaceMatrix =
                Multiply(skinCluster[meshIndex].inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
            mappedPalette[meshIndex][jointIndex].skeletonSpaceInverseTransposeMatrix =
                Transpose(Inverse(mappedPalette[meshIndex][jointIndex].skeletonSpaceMatrix));
        }

        ++meshIndex;
    }

    auto* commandList = DirectXBase::GetInstance()->GetCommandList().Get();
    commandList->SetComputeRootSignature(Object3dBase::GetInstance()->GetComputeRootSignature().Get());
    commandList->SetPipelineState(Object3dBase::GetInstance()->GetComputePipelineState().Get());

    SrvManager::GetInstance()->PreDraw();

    meshIndex = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        commandList->SetComputeRootDescriptorTable(0, SrvManager::GetInstance()->GetGPUDescriptorHandle(paletteSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(1, SrvManager::GetInstance()->GetGPUDescriptorHandle(inputVertexSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(2, SrvManager::GetInstance()->GetGPUDescriptorHandle(influenceSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(3, SrvManager::GetInstance()->GetGPUDescriptorHandle(outputVertexUavIndex[meshIndex]));
        commandList->SetComputeRootConstantBufferView(4, skinningInformationResource[meshIndex]->GetGPUVirtualAddress());
        commandList->Dispatch(UINT(matData.second.vertices.size() + 1023) / 1024, 1, 1); // 頂点数に応じてディスパッチサイズを計算

        D3D12_RESOURCE_BARRIER uavBarrier{};
        uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        uavBarrier.UAV.pResource = outputVertexResource[meshIndex].Get();
        commandList->ResourceBarrier(1, &uavBarrier);

        ++meshIndex;
    }
}

void Model::Draw() {

    int index = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(8, materialTemplateResource[matData.second.materialIndex]->GetGPUVirtualAddress());
        if (isAnimation)
        {
            DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &outputVertexBufferView[index]); // VBVを設定
        }
        else
        {
            DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView[0][index]); // VBVを設定
        }

        DirectXBase::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView.at(index)); // VBVを設定


        SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, modelData.material[matData.second.materialIndex].textureIndex);
        SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(10, modelData.material[matData.second.materialIndex].normalMapIndex); // ノーマルマップ
        SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(11, modelData.material[matData.second.materialIndex].metallicMapIndex); // メタリックマップ
        SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(12, modelData.material[matData.second.materialIndex].roughnessMapIndex); // ラフネスマップ

        SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(7, SkyBox::GetInstance()->GetSrvIndex());

        DirectXBase::GetInstance()->GetCommandList()->DrawIndexedInstanced(UINT(matData.second.indices.size()), 1, 0, 0, 0);

        index++;
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

void Model::CreateSkinningResources(const Skeleton& skeleton)
{
    if (!isAnimation)
    {
        return;
    }

    paletteResource.resize(modelData.matVertexData.size());
    inputVertexResource.resize(modelData.matVertexData.size());
    influenceResource.resize(modelData.matVertexData.size());
    outputVertexResource.resize(modelData.matVertexData.size());
    skinningInformationResource.resize(modelData.matVertexData.size());
    mappedPalette.resize(modelData.matVertexData.size());
    mappedInputVertex.resize(modelData.matVertexData.size());
    mappedInfluence.resize(modelData.matVertexData.size());
    mappedSkinningInformation.resize(modelData.matVertexData.size());
    outputVertexBufferView.resize(modelData.matVertexData.size());
    paletteSrvIndex.resize(modelData.matVertexData.size());
    inputVertexSrvIndex.resize(modelData.matVertexData.size());
    influenceSrvIndex.resize(modelData.matVertexData.size());
    outputVertexUavIndex.resize(modelData.matVertexData.size());

    size_t meshIndex = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        // MultiMeshごとに独立したGPUskinning用バッファとdescriptorを作成する。
        const size_t jointCount = skeleton.joints.size();
        const size_t vertexCount = matData.second.vertices.size();
        if (vertexCount == 0)
        {
            ++meshIndex;
            continue;
        }

        paletteResource[meshIndex] = DirectXBase::GetInstance()->CreateBufferResource(sizeof(WellForGPU) * jointCount);
        inputVertexResource[meshIndex] = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexData) * vertexCount);
        influenceResource[meshIndex] = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexInfluence) * vertexCount);
        outputVertexResource[meshIndex] = DirectXBase::GetInstance()->CreateUAVBufferResource(sizeof(VertexData) * vertexCount);
        skinningInformationResource[meshIndex] = DirectXBase::GetInstance()->CreateBufferResource(sizeof(SkinningInformation));

        paletteResource[meshIndex]->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette[meshIndex]));
        std::memset(mappedPalette[meshIndex], 0, sizeof(WellForGPU) * jointCount);

        inputVertexResource[meshIndex]->Map(0, nullptr, reinterpret_cast<void**>(&mappedInputVertex[meshIndex]));
        std::memcpy(mappedInputVertex[meshIndex], matData.second.vertices.data(), sizeof(VertexData) * vertexCount);

        influenceResource[meshIndex]->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence[meshIndex]));
        std::memset(mappedInfluence[meshIndex], 0, sizeof(VertexInfluence) * vertexCount);

        for (const auto& jointWeight : matData.second.skinClusterData)
        {
            auto it = skeleton.jointMap.find(jointWeight.first);
            if (it == skeleton.jointMap.end())
            {
                continue;
            }

            for (const auto& vertexWeight : jointWeight.second.vertexWeights)
            {
                if (vertexWeight.vertexIndex >= vertexCount)
                {
                    continue;
                }

                auto* currentInfluence = &mappedInfluence[meshIndex][vertexWeight.vertexIndex];
                for (uint32_t i = 0; i < numMaxInfluence; ++i)
                {
                    if (currentInfluence->weights[i] == 0.0f)
                    {
                        currentInfluence->weights[i] = vertexWeight.weight;
                        currentInfluence->jointIndices[i] = it->second;
                        break;
                    }
                }
            }
        }

        skinningInformationResource[meshIndex]->Map(0, nullptr, reinterpret_cast<void**>(&mappedSkinningInformation[meshIndex]));
        mappedSkinningInformation[meshIndex]->numVertices = static_cast<uint32_t>(vertexCount);

        outputVertexBufferView[meshIndex].BufferLocation = outputVertexResource[meshIndex]->GetGPUVirtualAddress();
        outputVertexBufferView[meshIndex].SizeInBytes = UINT(sizeof(VertexData) * vertexCount);
        outputVertexBufferView[meshIndex].StrideInBytes = sizeof(VertexData);

        paletteSrvIndex[meshIndex] = SrvManager::GetInstance()->Allocate();
        inputVertexSrvIndex[meshIndex] = SrvManager::GetInstance()->Allocate();
        influenceSrvIndex[meshIndex] = SrvManager::GetInstance()->Allocate();
        outputVertexUavIndex[meshIndex] = SrvManager::GetInstance()->Allocate();

        SrvManager::GetInstance()->CreateSRVforStructuredBuffer(paletteSrvIndex[meshIndex], paletteResource[meshIndex], static_cast<UINT>(jointCount), sizeof(WellForGPU));
        SrvManager::GetInstance()->CreateSRVforStructuredBuffer(inputVertexSrvIndex[meshIndex], inputVertexResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexData));
        SrvManager::GetInstance()->CreateSRVforStructuredBuffer(influenceSrvIndex[meshIndex], influenceResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexInfluence));
        SrvManager::GetInstance()->CreateUAVforStructuredBuffer(outputVertexUavIndex[meshIndex], outputVertexResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexData));

        ++meshIndex;
    }
}

void Model::AddAnimation(std::string directoryPath, std::string filename, std::string animationName)
{
    Animation anim = LoadAnimationFile(directoryPath, filename);
    animation[animationName] = anim;
}

void Model::AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& filenames)
{
    if (filenames.empty())
    {
        return;
    }

    auto animationNameFromFilename = [](const std::string& filename) {
        size_t start = filename.find_last_of("/\\");
        start = (start == std::string::npos) ? 0 : start + 1;
        size_t end = filename.find_last_of('.');
        if (end == std::string::npos || end < start)
        {
            end = filename.size();
        }
        return filename.substr(start, end - start);
        };

    std::vector<Animation> loadedAnimations(filenames.size());
    std::vector<std::thread> threads;
    threads.reserve(filenames.size());

    for (size_t i = 0; i < filenames.size(); i++)
    {
        // Assimpの読み込みはファイル単位で独立しているので並列化できる。
        threads.emplace_back([&, i]() {
            loadedAnimations[i] = LoadAnimationFile(directoryPath, filenames[i]);
            });
    }

    for (std::thread& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    for (size_t i = 0; i < filenames.size(); i++)
    {
        // スレッド側では共有mapを書き換えず、join後にメインスレッドで一括登録する。
        animation[animationNameFromFilename(filenames[i])] = loadedAnimations[i];
    }
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
            materialData.textureFilePath = "Resources/Sprite/white1x1.png";
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
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);
    //assert(scene->HasMeshes()); // メッシュが無いのは対応しない
    if (!scene->HasMeshes())
    {
        Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
        modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
        return modelData;
    }
    // ↑パスが間違ってる可能性(大)

    // マルチマテリアル対応のためにメモリを保管しておく
    //modelData.matVertexData.resize(scene->mNumMeshes);

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        // メッシュ名を取得(日本語に対応させるために変換)
        std::string utf8 = mesh->mName.C_Str();
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring meshName(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &meshName[0], len);
        modelData.matVertexData[meshName];

        //assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
        //assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応
        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
        {
            Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
            modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
            return modelData;
        }

        modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
        modelData.matVertexData[meshName].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管
        modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex;
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
            aiVector3D& tangent = mesh->mTangents[vertexIndex];
            aiVector3D& bitangent = mesh->mBitangents[vertexIndex];
            // 右手系->左手系への返還を忘れずに
            modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };

            modelData.matVertexData[meshName].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.matVertexData[meshName].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.matVertexData[meshName].vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };
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
                modelData.matVertexData[meshName].indices.push_back(vertexIndex);
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
            modelData.matVertexData[meshName].skinClusterData[JointName] = jointWeightData;
        }
    }
    uint32_t texIndex = TextureManager::GetInstance()->LoadTexture("Resources/Sprite/black1x1.png");
    // テクスチャが無い場合white1x1を張るようにする
    if (scene->mNumMaterials == 0)
    {
        MaterialData matData;
        matData.textureFilePath = "Resources/Sprite/white1x1.png";

        matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
        matData.normalMapIndex = texIndex;
        matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
        matData.metallicMapIndex = texIndex;
        matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
        matData.roughnessMapIndex = texIndex;

        // 読み込んだテクスチャの番号尾を取得
        matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);

        MaterialTemplate matTempData;
        // メタリックの数値
        matTempData.metallic = 0.0f;
        matTempData.roughness = 1.0f;

        modelData.materialTemplate.push_back(matTempData);
        modelData.material.push_back(matData);
    }
    // マテリアルを設定されている数読み込む
    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
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
            matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);


            // ノーマルマップ、メタリックマップ、ラフネスマップの読み込み
            if (material->GetTexture(aiTextureType_NORMALS, 0, &textureFilePath) == AI_SUCCESS ||
                material->GetTexture(aiTextureType_HEIGHT, 0, &textureFilePath) == AI_SUCCESS) {
                matData.normalMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.normalMapIndex = TextureManager::GetInstance()->LoadTexture(matData.normalMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
                matData.normalMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_METALNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.metallicMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.metallicMapIndex = TextureManager::GetInstance()->LoadTexture(matData.metallicMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
                matData.metallicMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.roughnessMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.roughnessMapIndex = TextureManager::GetInstance()->LoadTexture(matData.roughnessMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
                matData.roughnessMapIndex = texIndex;
            }


            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);


        }
        else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
        {
            MaterialData matData;
            matData.textureFilePath = "Resources/Sprite/white1x1.png";

            matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
            matData.normalMapIndex = texIndex;
            matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
            matData.metallicMapIndex = texIndex;
            matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
            matData.roughnessMapIndex = texIndex;

            // 読み込んだテクスチャの番号尾を取得
            matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);

            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);

        }
    }

    for (auto it = modelData.matVertexData.begin(); it != modelData.matVertexData.end();)
    {
        if (it->second.vertices.empty() || it->second.indices.empty())
        {
            Log("Model loader skipped an empty mesh.\n");
            it = modelData.matVertexData.erase(it);
            continue;
        }

        ++it;
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
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
    //assert(scene->HasMeshes()); 
    // メッシュが無いのは対応しない
    if (!scene->HasMeshes())
    {
        Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
        modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
        return modelData;
    }

    //modelData.matVertexData.resize(scene->mNumMeshes + 1);
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        // メッシュ名を取得(日本語に対応させるために変換)
        std::string utf8 = mesh->mName.C_Str();
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring meshName(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &meshName[0], len);
        modelData.matVertexData[meshName];

        //assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
        //assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応
        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
        {
            Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
            modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
            return modelData;
        }

        modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
        modelData.matVertexData[meshName].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管

        if (mesh->mMaterialIndex != 0)
        {
            modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex - 1;
        }
        else
        {
            modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex;
        }
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
            aiVector3D& tangent = mesh->mTangents[vertexIndex];
            aiVector3D& bitangent = mesh->mBitangents[vertexIndex];
            // 右手系->左手系への返還を忘れずに
            modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };

            modelData.matVertexData[meshName].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.matVertexData[meshName].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.matVertexData[meshName].vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };
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
                modelData.matVertexData[meshName].indices.push_back(vertexIndex);
            }
        }

    }
    uint32_t texIndex = TextureManager::GetInstance()->LoadTexture("Resources/Sprite/black1x1.png");
    // マテリアルが作成されていない場合はwhite1x1を使用
    if (scene->mNumMaterials == 1 || scene->mNumMaterials == 0)
    {
        MaterialData matData;
        matData.textureFilePath = "Resources/Sprite/white1x1.png";

        matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
        matData.normalMapIndex = texIndex;
        matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
        matData.metallicMapIndex = texIndex;
        matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
        matData.roughnessMapIndex = texIndex;

        // 読み込んだテクスチャの番号尾を取得
        matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);

        MaterialTemplate matTempData;
        // メタリックの数値		
        matTempData.metallic = 0.0f;
        matTempData.roughness = 1.0f;

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

            // 読み込んだテクスチャの番号尾を取得
            matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);

            // ノーマルマップ、メタリックマップ、ラフネスマップの読み込み
            if (material->GetTexture(aiTextureType_NORMALS, 0, &textureFilePath) == AI_SUCCESS ||
                material->GetTexture(aiTextureType_HEIGHT, 0, &textureFilePath) == AI_SUCCESS) {
                matData.normalMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.normalMapIndex = TextureManager::GetInstance()->LoadTexture(matData.normalMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
                matData.normalMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_METALNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.metallicMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.metallicMapIndex = TextureManager::GetInstance()->LoadTexture(matData.metallicMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
                matData.metallicMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.roughnessMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                matData.roughnessMapIndex = TextureManager::GetInstance()->LoadTexture(matData.roughnessMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
                matData.roughnessMapIndex = texIndex;
            }

            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);
        }
        else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
        {
            MaterialData matData;
            matData.textureFilePath = "Resources/Sprite/white1x1.png";

            matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
            matData.normalMapIndex = texIndex;
            matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
            matData.metallicMapIndex = texIndex;
            matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
            matData.roughnessMapIndex = texIndex;

            // 読み込んだテクスチャの番号尾を取得
            matData.textureIndex = TextureManager::GetInstance()->LoadTexture(matData.textureFilePath);

            MaterialTemplate matTempData;
            // メタリックの数値		
            matTempData.metallic = 0.0f;
            matTempData.roughness = 1.0f;

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);
        }
    }



    for (auto it = modelData.matVertexData.begin(); it != modelData.matVertexData.end();)
    {
        if (it->second.vertices.empty() || it->second.indices.empty())
        {
            Log("Model loader skipped an empty mesh.\n");
            it = modelData.matVertexData.erase(it);
            continue;
        }
        ++it;
    }
    modelData.rootNode = ReadNode(scene->mRootNode);
    return modelData;
}

void Model::CreateVertexResource() {
    vertexResource.resize(modelData.matVertexData.size());
    int index = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        // 頂点リソースの作成
        vertexResource.at(index) = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexData) * matData.second.vertices.size());
        index++;
    }
}

void Model::CreateVertexBufferView() {
    vertexBufferView[0].resize(modelData.matVertexData.size());
    int index = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        vertexBufferView[0][index].BufferLocation = vertexResource.at(index)->GetGPUVirtualAddress();
        vertexBufferView[0][index].SizeInBytes = UINT(sizeof(VertexData) * matData.second.vertices.size());
        vertexBufferView[0][index].StrideInBytes = sizeof(VertexData);
        index++;
    }
}

void Model::SetPBRMaterial(const float metallic, const float roughness) {
    const float clampedMetallic = std::clamp(metallic, 0.0f, 1.0f);
    const float clampedRoughness = std::clamp(roughness, 0.04f, 1.0f);

    for (size_t i = 0; i < modelData.materialTemplate.size(); ++i)
    {
        modelData.materialTemplate[i].metallic = clampedMetallic;
        modelData.materialTemplate[i].roughness = clampedRoughness;

        if (i < materialTemplateData.size() && materialTemplateData[i])
        {
            materialTemplateData[i]->metallic = clampedMetallic;
            materialTemplateData[i]->roughness = clampedRoughness;
        }
    }
}

void Model::CreateAABB() {
    if (modelData.vertices.empty() || modelData.matVertexData.empty())
    {
        meshAABB = {};
        multiMeshAABB.clear();
        return;
    }

    meshAABB.min.x = modelData.vertices[0].position.x;
    meshAABB.min.y = modelData.vertices[0].position.y;
    meshAABB.min.z = modelData.vertices[0].position.z;
    meshAABB.max.x = modelData.vertices[0].position.x;
    meshAABB.max.y = modelData.vertices[0].position.y;
    meshAABB.max.z = modelData.vertices[0].position.z;

    for (const auto& matVData : modelData.matVertexData)
    {
        if (matVData.second.vertices.empty())
        {
            continue;
        }

        AABB firstMultimesh;
        firstMultimesh.min.x = matVData.second.vertices[0].position.x;
        firstMultimesh.min.y = matVData.second.vertices[0].position.y;
        firstMultimesh.min.z = matVData.second.vertices[0].position.z;
        firstMultimesh.max.x = matVData.second.vertices[0].position.x;
        firstMultimesh.max.y = matVData.second.vertices[0].position.y;
        firstMultimesh.max.z = matVData.second.vertices[0].position.z;

        for (const auto& vertices : matVData.second.vertices)
        {
            firstMultimesh.min.x = std::min(firstMultimesh.min.x, vertices.position.x);
            firstMultimesh.min.y = std::min(firstMultimesh.min.y, vertices.position.y);
            firstMultimesh.min.z = std::min(firstMultimesh.min.z, vertices.position.z);

            firstMultimesh.max.x = std::max(firstMultimesh.max.x, vertices.position.x);
            firstMultimesh.max.y = std::max(firstMultimesh.max.y, vertices.position.y);
            firstMultimesh.max.z = std::max(firstMultimesh.max.z, vertices.position.z);
        }

        meshAABB.min.x = std::min(meshAABB.min.x, firstMultimesh.min.x);
        meshAABB.min.y = std::min(meshAABB.min.y, firstMultimesh.min.y);
        meshAABB.min.z = std::min(meshAABB.min.z, firstMultimesh.min.z);

        meshAABB.max.x = std::max(meshAABB.max.x, firstMultimesh.max.x);
        meshAABB.max.y = std::max(meshAABB.max.y, firstMultimesh.max.y);
        meshAABB.max.z = std::max(meshAABB.max.z, firstMultimesh.max.z);

        multiMeshAABB[matVData.first] = firstMultimesh;
    }
}
