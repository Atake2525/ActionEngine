#include <wrl.h>
#include <vector>
#include <d3d12.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include <optional>
#include <map>
#include "ModelLoader.h"

#pragma once



class DirectXBase;
class SrvManager;
class TextureManager;
class Object3dBase;
class SkyBox;

class Model {
public:
	struct SkinningInformation
	{
		uint32_t numVertices;
	};

	void SetContext(DirectXBase& directXBase, SrvManager& srvManager, TextureManager& textureManaeger, Object3dBase& object3dBase, SkyBox& skyBox);

	// 初期化
	void Initialize(std::string directoryPath, std::string fileName, bool isAnimation);

	void SkinningUpdate(const Skeleton& skeleton);

	// 更新
	void Draw();

	// Getter(Animation)
	const std::unordered_map<std::string, Animation>& GetAnimation() const { return animation; }
	const Animation& GetDefaultAnimation() const { return animation.at("DefaultAnimation"); }
	const bool& IsAnimation() const { return isAnimation; }

	const ModelData& GetModelData() const { return modelData; }
	// Getter(ModelData vertices)
	const std::vector<VertexData>& GetVertices() const { return modelData.vertices; }

	// SkinClusterのセット(通常使うものではないため気にしないで良い)
	void SetSkinCluster(const std::vector<SkinCluster> skinCluster);
	void CreateSkinningResources(const Skeleton& skeleton);

	// アニメーションの追加
	void AddAnimation(std::string directoryPath, std::string fileName, std::string animationName);

	void AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& fileNames);

	void SetPBRMaterial(const float metallic, const float roughness);

	// AABBの取得
	const AABB& GetMeshAABB() const { return meshAABB; }
	const std::unordered_map<std::wstring, AABB>& GetMultiMeshAABB() const { return multiMeshAABB; }

private:
	DirectXBase* m_pDirectXBase = nullptr;
	SrvManager* m_pSrvManager = nullptr;
	TextureManager* m_pTextureManager = nullptr;
	Object3dBase* m_pObject3dBase = nullptr;
	SkyBox* m_pSkyBox = nullptr;


	std::unordered_map<std::string, Animation> animation;
	bool isAnimation = false;

	std::vector<SkinCluster> skinCluster;

	//Skelton skelton;

	// 頂点データのバッファリソース
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource;
	// 頂点データのバッファリソース内のデータを指すポインタ
	std::vector<VertexData*> vertexData;
	// バッファリソースの使い道を指定するバッファビュー
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView[2];

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResource;

	std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferView = {};

	uint32_t* mappedIndex = nullptr;

	// Objファイルのデータ
	ModelData modelData;

	// マテリアルのバッファリソース
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialTemplateResource;
	// マテリアルバッファリソース内のデータを指すポインタ
	std::vector<MaterialTemplate*> materialTemplateData;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> paletteResource;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> inputVertexResource;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> influenceResource;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> outputVertexResource;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> skinningInformationResource;

	std::vector<WellForGPU*> mappedPalette;
	std::vector<VertexData*> mappedInputVertex;
	std::vector<VertexInfluence*> mappedInfluence;
	std::vector<SkinningInformation*> mappedSkinningInformation;

	std::vector<D3D12_VERTEX_BUFFER_VIEW> outputVertexBufferView{};

	std::vector<uint32_t> paletteSrvIndex;
	std::vector<uint32_t> inputVertexSrvIndex;
	std::vector<uint32_t> influenceSrvIndex;
	std::vector<uint32_t> outputVertexUavIndex;

private:
	// .mtlファイルの読み取り
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName);
	void LoadModelTexture();
	// VertexResourceを作成する
	void CreateVertexResource();
	// VertexBufferViewを作成する(値を設定するだけ)
	void CreateVertexBufferView();

	// ModelのAABBを作成する
	void CreateAABB();


	AABB meshAABB;
	std::unordered_map<std::wstring, AABB> multiMeshAABB;

public:

	Model() = default;
};
