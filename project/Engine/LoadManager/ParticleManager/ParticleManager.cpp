#include "ParticleManager.h"
#include "DirectXBase.h"
#include <cassert>
#include "Logger.h"
#include "TextureManager.h"
#include "kMath.h"
#include "Camera.h"
#include "SrvManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


using namespace Logger;

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr)
	{
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Finalize() {
	particleGroups.clear();
	delete instance;
	instance = nullptr;
}

void ParticleManager::Initialize(DirectXBase* directxBase) {
	directxBase_ = directxBase;
	InitializeRandomEngine();
	CreateGraphicsPipeLineState();
}

void ParticleManager::CreateParticleGroupFromOBJ(std::string directoryPath, std::string filename, const std::string& name) {
	if (particleGroups.contains(name))
	{
		// 読み込み済なら早期return
		return;
	}

	// パーティクルの作成
	ParticleGroup group;
	group.modelData = LoadModelFile(directoryPath, filename);
	// callData(Resource)などが入った構造体をリサイズする
	group.callData.resize(group.modelData.matVertexData.size());

	group.numInstance = 0;
	group.particleName = name;
	for (size_t i = 0; i < group.modelData.matVertexData.size(); i++)
	{
#pragma region InitializeMaterialResource

		group.materialResource = directxBase_->CreateBufferResource(sizeof(Material));

		//  書き込むためのアドレスを取得
		group.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&group.material));

		group.material->color = { 1.0f, 1.0f, 1.0f, 1.0f };

		group.material->uvTransform = MakeIdentity4x4();

		group.material->enableLighting = false;
		group.material->shininess = 70.0f;
		group.material->specularColor = { 1.0f, 1.0f, 1.0f };

#pragma endregion


#pragma region InitializeVertexResource

		size_t size = sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size();

		// VertexResourceの初期化
		group.callData[i].vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());

		group.callData[i].vertexBufferView.BufferLocation = group.callData[i].vertexResource->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点6つ分のサイズ
		group.callData[i].vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());
		// 1頂点あたりのサイズ
		group.callData[i].vertexBufferView.StrideInBytes = sizeof(VertexData);

		// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
		group.callData[i].vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].vertexData));
		std::memcpy(group.callData[i].vertexData, group.modelData.matVertexData[i].vertices.data(), sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());

#pragma endregion


#pragma region InitializeIndexResource

		/*size = sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size();

		group.callData[i].indexResource = directxBase_->CreateBufferResource(sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());

		group.callData[i].indexBufferView.BufferLocation = group.callData[i].indexResource->GetGPUVirtualAddress();
		group.callData[i].indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());
		group.callData[i].indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		group.callData[i].indexResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].mappedIndex));
		std::memcpy(&group.callData[i].mappedIndex, group.modelData.matVertexData[i].indices.data(), sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());*/

#pragma endregion


		size = sizeof(ParticleForGPU) * maxNumInstance;

		group.callData[i].instancingResource = directxBase_->CreateBufferResource(sizeof(ParticleForGPU) * maxNumInstance);
		group.callData[i].instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].instancingData));
		for (uint32_t index = 0; index < group.numInstance; index++)
		{
			group.callData[i].instancingData[index].WVP = MakeIdentity4x4();
			group.callData[i].instancingData[index].World = MakeIdentity4x4();
		}
	}
	group.particleFlag.isAccelerationField = false;
	group.particleFlag.start = false;

	//group.accelerationField.acceleration = { 0.0f, 0.0f, 0.0f };
	//group.accelerationField.area = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	group.accelerationField.acceleration = { 15.0f, 0.0f, 0.0f };
	group.accelerationField.area.min = { -1.0f, -1.0f, -1.0f };
	group.accelerationField.area.max = { 1.0f, 1.0f, 1.0f };

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = maxNumInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	for (size_t i = 0; i < group.modelData.matVertexData.size(); i++)
	{
		group.callData[i].srvIndex = SrvManager::GetInstance()->Allocate();

		assert(SrvManager::GetInstance()->CheckAllocate());

		D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(group.callData[i].srvIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(group.callData[i].srvIndex);
		directxBase_->GetDevice()->CreateShaderResourceView(group.callData[i].instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);
	}

	group.instancingSrvDesc = instancingSrvDesc;

	particleGroups[name] = group;

}

void ParticleManager::CreateParticleGroup(ParticleType particleType, std::string textureFilePath, const std::string& name)
{
	if (particleGroups.contains(name))
	{
		// 読み込み済なら早期return
		return;
	}


	// パーティクルの作成
	ParticleGroup group;

	switch (particleType)
	{
	case ParticleType::plane:
		group.modelData = CreatePlaneModel();
		break;
	case ParticleType::Ring:

		break;
	case ParticleType::Cylinder:

		break;
	default:
		break;
	}

	MaterialData materialData;
	materialData.textureFilePath = textureFilePath;
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);
	group.modelData.material.push_back(materialData);
	// callData(Resource)などが入った構造体をリサイズする
	group.callData.resize(group.modelData.matVertexData.size());

	group.numInstance = 0;
	group.particleName = name;
	for (size_t i = 0; i < group.modelData.matVertexData.size(); i++)
	{
#pragma region InitializeMaterialResource

		group.materialResource = directxBase_->CreateBufferResource(sizeof(Material));

		//  書き込むためのアドレスを取得
		group.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&group.material));

		group.material->color = { 1.0f, 1.0f, 1.0f, 1.0f };

		group.material->uvTransform = MakeIdentity4x4();

		group.material->enableLighting = false;
		group.material->shininess = 70.0f;
		group.material->specularColor = { 1.0f, 1.0f, 1.0f };

#pragma endregion


#pragma region InitializeVertexResource

		size_t size = sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size();

		// VertexResourceの初期化
		group.callData[i].vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());

		group.callData[i].vertexBufferView.BufferLocation = group.callData[i].vertexResource->GetGPUVirtualAddress();
		// 使用するリソースのサイズは頂点6つ分のサイズ
		group.callData[i].vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());
		// 1頂点あたりのサイズ
		group.callData[i].vertexBufferView.StrideInBytes = sizeof(VertexData);

		// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
		group.callData[i].vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].vertexData));
		std::memcpy(group.callData[i].vertexData, group.modelData.matVertexData[i].vertices.data(), sizeof(VertexData) * group.modelData.matVertexData[i].vertices.size());

#pragma endregion


#pragma region InitializeIndexResource

		/*size = sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size();

		group.callData[i].indexResource = directxBase_->CreateBufferResource(sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());

		group.callData[i].indexBufferView.BufferLocation = group.callData[i].indexResource->GetGPUVirtualAddress();
		group.callData[i].indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());
		group.callData[i].indexBufferView.Format = DXGI_FORMAT_R32_UINT;

		group.callData[i].indexResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].mappedIndex));
		std::memcpy(&group.callData[i].mappedIndex, group.modelData.matVertexData[i].indices.data(), sizeof(uint32_t) * group.modelData.matVertexData[i].indices.size());*/

#pragma endregion


		size = sizeof(ParticleForGPU) * maxNumInstance;

		group.callData[i].instancingResource = directxBase_->CreateBufferResource(sizeof(ParticleForGPU) * maxNumInstance);
		group.callData[i].instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group.callData[i].instancingData));
		for (uint32_t index = 0; index < group.numInstance; index++)
		{
			group.callData[i].instancingData[index].WVP = MakeIdentity4x4();
			group.callData[i].instancingData[index].World = MakeIdentity4x4();
		}
	}
	group.particleFlag.isAccelerationField = false;
	group.particleFlag.start = false;

	//group.accelerationField.acceleration = { 0.0f, 0.0f, 0.0f };
	//group.accelerationField.area = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	group.accelerationField.acceleration = { 15.0f, 0.0f, 0.0f };
	group.accelerationField.area.min = { -1.0f, -1.0f, -1.0f };
	group.accelerationField.area.max = { 1.0f, 1.0f, 1.0f };

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = maxNumInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	for (size_t i = 0; i < group.modelData.matVertexData.size(); i++)
	{
		group.callData[i].srvIndex = SrvManager::GetInstance()->Allocate();

		assert(SrvManager::GetInstance()->CheckAllocate());

		D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(group.callData[i].srvIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(group.callData[i].srvIndex);
		directxBase_->GetDevice()->CreateShaderResourceView(group.callData[i].instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);
	}

	group.instancingSrvDesc = instancingSrvDesc;

	particleGroups[name] = group;
}

Particle ParticleManager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Particle particle;
	particle.transform.scale = { 0.5f, 0.5f, 0.5f };
	particle.transform.rotate = { 0.0f, 3.14f, 0.0f };
	particle.transform.translate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };

	Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.transform.translate = translate + randomTranslate;

	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine), 1.0f };

	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

Particle ParticleManager::MakeNewParticle_HitEffect(std::mt19937& randomEngine, const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distributionRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	Particle particle;
	particle.transform.scale = { 0.025f, 0.5f, 0.5f };
	particle.transform.rotate = { 0.0f, 0.0f, distributionRotate(randomEngine)};
	particle.transform.translate = { translate };
	//particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { 0.0f, 0.0f, 0.0f };


	//Vector3 randomTranslate{ distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	//particle.transform.translate = translate + randomTranslate;

	//std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	//std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	particle.lifeTime = 1.0f;
	particle.currentTime = 0;

	return particle;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count) {
	// 登録済みのパーティクルグループかチェックしてassert
	//auto it = particleGroups.find(name);
	//if (it == particleGroups.end())
	//{
	//	assert(false);
	//	// 読み込み済じゃないなら早期return
	//	return;
	//}
	if (!particleGroups.contains(name))
	{
		assert(false);
		// 読み込み済じゃないなら早期return
		return;
	}
	std::list<Particle> particles;
	//particles.transform.translate = position;
	for (uint32_t con = 0; con < count; ++con) {
		particleGroups[name].particles.push_back(MakeNewParticle(randomEngine, position));
	}
	//it->second.particle.splice(particles.end(), particles);
	//particleGroups[name].particles.resize(count);
	//particleGroups[name].particles.insert(particles);
	//it->second.particles.resize(count);
	//it->second.particles.splice(particles.end(), particles);

}

void ParticleManager::Update() {
	
	// CG3_01_02
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	// 一旦常にBillboardするようにしておく
	//if (!useBillboard) {
	billboardMatrix = MakeIdentity4x4();
	//}

	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroup = particleGroups.begin(); particleGroup != particleGroups.end(); ++particleGroup)
	{
		particleGroup->second.numInstance = 0;
		for (std::list<Particle>::iterator particleIterator = particleGroup->second.particles.begin(); particleIterator != particleGroup->second.particles.end();) {
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup->second.particles.erase(particleIterator); // 生存時間が過ぎたParticleはlistから消す。戻り値が次のイテレータとなる
				//particleGroup->second.numInstance--;
				continue;
			}
			// Fieldの範囲内のParticleには加速度を適用する
			if (particleGroup->second.particleFlag.isAccelerationField) {
				if (IsCollision(particleGroup->second.accelerationField.area, (*particleIterator).transform.translate)) {
					(*particleIterator).velocity += particleGroup->second.accelerationField.acceleration * deltaTime;
				}
			}
			//(*particleIterator).currentTime = (*particleIterator).currentTime;
			//(*particleIterator).currentTime += deltaTime;
			(*particleIterator).currentTime += deltaTime;
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			(*particleIterator).transform.translate += (*particleIterator).velocity * deltaTime;
			//if (particleGroup->second.particleFlag.start) {
			//	// ...WorldMatrixを求めたり
			//	//alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			//}

			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
			//billboardMatrix = MakeRotateZMatrix((*particleIterator).transform.rotate.z);
			//Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));
			Matrix4x4 worldMatrix = MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);
			const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			// インスタンスが最大数を超えないようにする
			for (size_t i = 0; i < particleGroup->second.callData.size(); i++)
			{
				if (particleGroup->second.numInstance < maxNumInstance) {
					particleGroup->second.callData[i].instancingData[particleGroup->second.numInstance].WVP = worldViewProjectionMatrix;
					particleGroup->second.callData[i].instancingData[particleGroup->second.numInstance].World = worldMatrix;
					particleGroup->second.callData[i].instancingData[particleGroup->second.numInstance].color = (*particleIterator).color;
					particleGroup->second.callData[i].instancingData[particleGroup->second.numInstance].color.w = alpha;
					++particleGroup->second.numInstance;
				}
			}
			++particleIterator;
		}
	}
}

void ParticleManager::Draw() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directxBase_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	directxBase_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get()); 
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	directxBase_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 全パーティクルについての処理
	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroup = particleGroups.begin(); particleGroup != particleGroups.end(); ++particleGroup)
	{
		// 描画に必要なときのみ以下の処理を行うようにする
		if (particleGroup->second.numInstance > 0)
		{
			directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleGroup->second.materialResource->GetGPUVirtualAddress());

			for (size_t i = 0; i < particleGroup->second.modelData.matVertexData.size(); i++)
			{
				// VBVを設定
				directxBase_->GetCommandList()->IASetVertexBuffers(0, 1, &particleGroup->second.callData[i].vertexBufferView);

				//directxBase_->GetCommandList()->IASetIndexBuffer(&particleGroup->second.callData[i].indexBufferView);

				// インスタンシングデータのSRVのDescriptorTableを設定
				SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(1, particleGroup->second.callData[i].srvIndex);


				// テクスチャのSRVのDescriptorTableを設定
				SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, particleGroup->second.modelData.material[i].textureIndex);

				// DrawCall
				//directxBase_->GetCommandList()->DrawIndexedInstanced(UINT(modelData.matVertexData[i].indices.size()), particleGroup->second.numInstance, 0, 0, 0);
				directxBase_->GetCommandList()->DrawInstanced(UINT(particleGroup->second.modelData.matVertexData[i].vertices.size()), particleGroup->second.numInstance, 0, 0);
			}
		}
	}
}

void ParticleManager::InitializeRandomEngine() {
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
}

void ParticleManager::CreateRootSignature() {
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRange[0].NumDescriptors = 1;    // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	descriptorRangeForInstancing[0].BaseShaderRegister = 0; // 0から始める
	descriptorRangeForInstancing[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Samplerの設定
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;   // バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;     // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                       // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;                               // レジスタ番号0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                              // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescroptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;          // VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;            // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = directxBase_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	// InputLayout
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// NomalBlendを行うための設定
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	// RasiterzerStateの設定
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	vertexShaderBlob = directxBase_->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = directxBase_->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void ParticleManager::CreateGraphicsPipeLineState() {
	CreateRootSignature();
	// PSOを作成する
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();                                           // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                  // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };   // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                         // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                               // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ、三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	HRESULT hr = directxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

}

ModelData ParticleManager::CreatePlaneModel()
{
	ModelData model;
	VertexData vData;

	model.matVertexData.resize(1);

	vData = {
		{-1.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f}
	};
	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);

	vData = {
		{1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f},
		{0.0f, 0.0f, 1.0f}
	};

	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);

	vData = {
		{-1.0f, -1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);

	vData = {
		{1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f},
		{0.0f, 0.0f, 1.0f}
	};

	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);

	vData = {
		{1.0f, -1.0f, 0.0f, 1.0f},
		{1.0f, -1.0f},
		{0.0f, 0.0f, 1.0f}
	};

	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);

	vData = {
		{-1.0f, -1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	model.vertices.push_back(vData);
	model.matVertexData[0].vertices.push_back(vData);
	return model;
}

//void ParticleManager::InitializeVetexData() {
//	modelData = LoadModelFile("Resources/Model/obj", "plane.obj");
//}

//Microsoft::WRL::ComPtr<ID3D12Resource> ParticleManager::InitializeVertexResource(size_t vertexSize)
//{
//	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
//	vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * vertexSize);
//	return vertexResource;
//}

// マルチスレッド化予定
ModelData ParticleManager::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene->HasMeshes()); // メッシュが無いのは対応しない

	modelData.matVertexData.resize(scene->mNumMeshes + 1);
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応

		modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
		modelData.matVertexData[meshIndex].vertices.resize(mesh->mNumVertices);
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
			assert(face.mNumIndices == 3);

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
	return modelData;
}

//void ParticleManager::CreateVertexResource() {
//	// 頂点リソースの作成
//	vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
//}

//void ParticleManager::CreateVertexxBufferView() {
//	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
//	// 使用するリソースのサイズは頂点6つ分のサイズ
//	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
//	// 1頂点あたりのサイズ
//	vertexBufferView.StrideInBytes = sizeof(VertexData);
//}
//
//void ParticleManager::MappingVertexData() {
//	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
//	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
//	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
//}
//
//void ParticleManager::CreateMaterialResource() {
//	materialResource = directxBase_->CreateBufferResource(sizeof(Material));
//}

bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point) {
	if ((aabb.min.x <= point.x && aabb.max.x >= point.x) &&
		(aabb.min.y <= point.y && aabb.max.y >= point.y) &&
		(aabb.min.z <= point.z && aabb.max.z >= point.z)) {
		return true;
	}
	return false;
}
