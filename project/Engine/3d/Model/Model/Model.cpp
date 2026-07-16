#include "Model.h"
#include "DirectXBase.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "Object3dBase.h"
#include "SkyBox.h"
#include "json.hpp"
#include "kMath.h"
#include "Logger.h"
#include <thread>
#include <algorithm>

using namespace Logger;

void Model::SetContext(DirectXBase& directXBase, SrvManager& srvManager, TextureManager& textureManaeger, Object3dBase& object3dBase, SkyBox& skyBox) {
    m_pDirectXBase = &directXBase;
    m_pSrvManager = &srvManager;
    m_pTextureManager = &textureManaeger;
    m_pObject3dBase = &object3dBase;
    m_pSkyBox = &skyBox;
}

void Model::Initialize(std::string directoryPath, std::string fileName, bool isAnimation) {
    bool enableAnimationLoad = isAnimation;

    if (fileName.ends_with(".obj"))
    {
        // モデル読み込み
        modelData = ModelLoader::LoadModelFileOBJ(directoryPath, fileName);
    }
    else if (fileName.ends_with(".gltf"))
    {// モデル読み込み
        modelData = ModelLoader::LoadModelFileGLTF(directoryPath, fileName);
    }
    LoadModelTexture();

    if (isAnimation)
    {
        this->isAnimation = enableAnimationLoad;
        Animation anim = ModelLoader::LoadAnimationFile(directoryPath, fileName);
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

        indexResource.at(num) = m_pDirectXBase->CreateBufferResource(sizeof(uint32_t) * matData.second.indices.size());

        indexBufferView.at(num).BufferLocation = indexResource.at(num)->GetGPUVirtualAddress();
        indexBufferView.at(num).SizeInBytes = UINT(sizeof(uint32_t) * matData.second.indices.size());
        indexBufferView.at(num).Format = DXGI_FORMAT_R32_UINT;

        indexResource.at(num)->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
        std::memcpy(mappedIndex, matData.second.indices.data(), sizeof(uint32_t) * matData.second.indices.size());

        materialTemplateResource.at(matData.second.materialIndex) = m_pDirectXBase->CreateBufferResource(sizeof(MaterialTemplate));
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

        const size_t jointCount = (std::min)(skeleton.joints.size(), skinCluster[meshIndex].inverseBindPoseMatrices.size());
        for (size_t jointIndex = 0; jointIndex < jointCount; ++jointIndex)
        {
            mappedPalette[meshIndex][jointIndex].skeletonSpaceMatrix =
                Multiply(skinCluster[meshIndex].inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
            mappedPalette[meshIndex][jointIndex].skeletonSpaceInverseTransposeMatrix =
                Transpose(Inverse(mappedPalette[meshIndex][jointIndex].skeletonSpaceMatrix));
        }

        ++meshIndex;
    }

    auto* commandList = m_pDirectXBase->GetCommandList().Get();
    commandList->SetComputeRootSignature(m_pObject3dBase->GetComputeRootSignature().Get());
    commandList->SetPipelineState(m_pObject3dBase->GetComputePipelineState().Get());

    m_pSrvManager->PreDraw();

    meshIndex = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        commandList->SetComputeRootDescriptorTable(0, m_pSrvManager->GetGPUDescriptorHandle(paletteSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(1, m_pSrvManager->GetGPUDescriptorHandle(inputVertexSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(2, m_pSrvManager->GetGPUDescriptorHandle(influenceSrvIndex[meshIndex]));
        commandList->SetComputeRootDescriptorTable(3, m_pSrvManager->GetGPUDescriptorHandle(outputVertexUavIndex[meshIndex]));
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
        m_pDirectXBase->GetCommandList()->SetGraphicsRootConstantBufferView(8, materialTemplateResource[matData.second.materialIndex]->GetGPUVirtualAddress());
        if (isAnimation)
        {
            m_pDirectXBase->GetCommandList()->IASetVertexBuffers(0, 1, &outputVertexBufferView[index]); // VBVを設定
        }
        else
        {
            m_pDirectXBase->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView[0][index]); // VBVを設定
        }

        m_pDirectXBase->GetCommandList()->IASetIndexBuffer(&indexBufferView.at(index)); // VBVを設定


        m_pSrvManager->SetGraphicsRootDescriptorTable(2, modelData.material[matData.second.materialIndex].textureIndex);
        m_pSrvManager->SetGraphicsRootDescriptorTable(10, modelData.material[matData.second.materialIndex].normalMapIndex); // ノーマルマップ
        m_pSrvManager->SetGraphicsRootDescriptorTable(11, modelData.material[matData.second.materialIndex].metallicMapIndex); // メタリックマップ
        m_pSrvManager->SetGraphicsRootDescriptorTable(12, modelData.material[matData.second.materialIndex].roughnessMapIndex); // ラフネスマップ

        m_pSrvManager->SetGraphicsRootDescriptorTable(7, m_pSkyBox->GetSrvIndex());

        m_pDirectXBase->GetCommandList()->DrawIndexedInstanced(UINT(matData.second.indices.size()), 1, 0, 0, 0);

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

        paletteResource[meshIndex] = m_pDirectXBase->CreateBufferResource(sizeof(WellForGPU) * jointCount);
        inputVertexResource[meshIndex] = m_pDirectXBase->CreateBufferResource(sizeof(VertexData) * vertexCount);
        influenceResource[meshIndex] = m_pDirectXBase->CreateBufferResource(sizeof(VertexInfluence) * vertexCount);
        outputVertexResource[meshIndex] = m_pDirectXBase->CreateUAVBufferResource(sizeof(VertexData) * vertexCount);
        skinningInformationResource[meshIndex] = m_pDirectXBase->CreateBufferResource(sizeof(SkinningInformation));

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

        paletteSrvIndex[meshIndex] = m_pSrvManager->Allocate();
        inputVertexSrvIndex[meshIndex] = m_pSrvManager->Allocate();
        influenceSrvIndex[meshIndex] = m_pSrvManager->Allocate();
        outputVertexUavIndex[meshIndex] = m_pSrvManager->Allocate();

        m_pSrvManager->CreateSRVforStructuredBuffer(paletteSrvIndex[meshIndex], paletteResource[meshIndex], static_cast<UINT>(jointCount), sizeof(WellForGPU));
        m_pSrvManager->CreateSRVforStructuredBuffer(inputVertexSrvIndex[meshIndex], inputVertexResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexData));
        m_pSrvManager->CreateSRVforStructuredBuffer(influenceSrvIndex[meshIndex], influenceResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexInfluence));
        m_pSrvManager->CreateUAVforStructuredBuffer(outputVertexUavIndex[meshIndex], outputVertexResource[meshIndex], static_cast<UINT>(vertexCount), sizeof(VertexData));

        ++meshIndex;
    }
}

void Model::AddAnimation(std::string directoryPath, std::string fileName, std::string animationName)
{
    Animation anim = ModelLoader::LoadAnimationFile(directoryPath, fileName);
    animation[animationName] = anim;
}

void Model::AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& fileNames)
{
    if (fileNames.empty())
    {
        return;
    }

    auto animationNameFromFileName = [](const std::string& fileName) {
        size_t start = fileName.find_last_of("/\\");
        start = (start == std::string::npos) ? 0 : start + 1;
        size_t end = fileName.find_last_of('.');
        if (end == std::string::npos || end < start)
        {
            end = fileName.size();
        }
        return fileName.substr(start, end - start);
        };

    std::vector<Animation> loadedAnimations(fileNames.size());
    std::vector<std::thread> threads;
    threads.reserve(fileNames.size());

    for (size_t i = 0; i < fileNames.size(); i++)
    {
        // Assimpの読み込みはファイル単位で独立しているので並列化できる。
        threads.emplace_back([&, i]() {
            loadedAnimations[i] = ModelLoader::LoadAnimationFile(directoryPath, fileNames[i]);
            });
    }

    for (std::thread& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    for (size_t i = 0; i < fileNames.size(); i++)
    {
        // スレッド側では共有mapを書き換えず、join後にメインスレッドで一括登録する。
        animation[animationNameFromFileName(fileNames[i])] = loadedAnimations[i];
    }
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName) {
    // 1, 中で必要となる変数の宣言
    MaterialData materialData; // 構築するMaterialData
    std::string line;          // ファイルから読んだ１行を格納するもの
    // 2, ファイルを開く
    std::ifstream file(directoryPath + "/" + fileName); // ファイルを開く
    assert(file.is_open());                             // とりあえず開けなかったら止める
    // 3, 実際にファイルを読み、MaterialDataを構築していく
    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        // identifierに応じた処理
        if (identifier == "map_Kd") {
            std::string textureFileName;
            s >> textureFileName;
            // 連結してファイルパスにする
            materialData.textureFilePath = directoryPath + "/" + textureFileName;
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



void Model::CreateVertexResource() {
    vertexResource.resize(modelData.matVertexData.size());
    int index = 0;
    for (const auto& matData : modelData.matVertexData)
    {
        // 頂点リソースの作成
        vertexResource.at(index) = m_pDirectXBase->CreateBufferResource(sizeof(VertexData) * matData.second.vertices.size());
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
            firstMultimesh.min.x = (std::min)(firstMultimesh.min.x, vertices.position.x);
            firstMultimesh.min.y = (std::min)(firstMultimesh.min.y, vertices.position.y);
            firstMultimesh.min.z = (std::min)(firstMultimesh.min.z, vertices.position.z);

            firstMultimesh.max.x = (std::max)(firstMultimesh.max.x, vertices.position.x);
            firstMultimesh.max.y = (std::max)(firstMultimesh.max.y, vertices.position.y);
            firstMultimesh.max.z = (std::max)(firstMultimesh.max.z, vertices.position.z);
        }

        meshAABB.min.x = (std::min)(meshAABB.min.x, firstMultimesh.min.x);
        meshAABB.min.y = (std::min)(meshAABB.min.y, firstMultimesh.min.y);
        meshAABB.min.z = (std::min)(meshAABB.min.z, firstMultimesh.min.z);

        meshAABB.max.x = (std::max)(meshAABB.max.x, firstMultimesh.max.x);
        meshAABB.max.y = (std::max)(meshAABB.max.y, firstMultimesh.max.y);
        meshAABB.max.z = (std::max)(meshAABB.max.z, firstMultimesh.max.z);

        multiMeshAABB[matVData.first] = firstMultimesh;
    }
}

void Model::LoadModelTexture() {
    for (auto& matData : modelData.material)
    {
        if (!matData.textureFilePath.empty())
        {
            matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);
        }
        if (!matData.normalMapFilePath.empty())
        {
            matData.normalMapIndex = m_pTextureManager->LoadTexture(matData.normalMapFilePath);
        }
        if (!matData.metallicMapFilePath.empty())
        {
            matData.metallicMapIndex = m_pTextureManager->LoadTexture(matData.metallicMapFilePath);
        }
        if (!matData.roughnessMapFilePath.empty())
        {
            matData.roughnessMapIndex = m_pTextureManager->LoadTexture(matData.roughnessMapFilePath);
        }
    }
}