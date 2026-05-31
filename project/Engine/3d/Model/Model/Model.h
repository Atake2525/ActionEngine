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
#include "Animator.h"
#include "Transform.h"
#include <optional>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma once

// 頂点情報を格納するための構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;
};

// マルチメッシュ、マルチマテリアル用の各種情報を格納するための構造体
struct MaterialVertexData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	std::map<std::string, JointWeightData> skinClusterData;
	size_t materialIndex;
};

// マテリアル
struct Material {
	Vector4 color;

	int32_t enableLighting;
	int pad0[3];

	Matrix4x4 uvTransform;

	float shininess;
	//float pad1[3];

	Vector3 specularColor;
	//float pad3;

	uint32_t enableMetallic;
	float environmentCoefficient;
	float pad2[2];
};

// マテリアル
struct MaterialTemplate
{
	float metallic;
	float roughness;
	float padding[2];
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;

	std::string normalMapFilePath;
	uint32_t normalMapIndex = 0;

	std::string metallicMapFilePath;
	uint32_t metallicMapIndex = 0;

	std::string roughnessMapFilePath;
	uint32_t roughnessMapIndex = 0;
};


struct ModelData {
	std::map<std::string, JointWeightData> skinClusterData;
	std::vector<uint32_t> indices;
	std::vector<VertexData> vertices;
	std::map<std::wstring, MaterialVertexData> matVertexData;
	std::vector<MaterialData> material;
	std::vector<MaterialTemplate> materialTemplate;
	Node rootNode;
};

class Model {
public:
	struct SkinningInformation
	{
		uint32_t numVertices;
	};

	// 初期化
	void Initialize(std::string directoryPath, std::string filename, bool enableLighting, bool isAnimation);

    void SkinningUpdate(const Skeleton& skeleton);

	// 更新
	void Draw();


	// Getter(Color)
	const Vector4& GetColor() const { return materialData->color; }
	// Getter(EnableLighting)
	const bool GetEnableLighting() const { return materialData->enableLighting; }
	const float& GetEnvironmentCoefficient() const { return materialData->environmentCoefficient; }
	// Getter(SpecularColor)
	const Vector3& GetSpecularColor() const { return materialData->specularColor; }
	// Getter(Shininess)
	const float& GetShininess() const { return materialData->shininess; }
	// Getter(ModelData)
	const ModelData& GetModelData() const { return modelData; }
	// Getter(Animation)
	const std::map<std::string, Animation>& GetAnimation() const { return animation; }
	const Animation& GetDefaultAnimation() const { return animation.at("DefaultAnimation"); }
	const bool& IsAnimation() const { return isAnimation; }

	// Getter(ModelData vertices)
	const std::vector<VertexData>& GetVertices() const { return modelData.vertices; }

	// Setter(Color)
	void SetColor(const Vector4 color) {
		materialData->color = color;
	}
	// Setter(EnableLighting)
	void SetEnableLighting(const bool enableLighting) {
		materialData->enableLighting = enableLighting;
	}
	// Setter(Shininess)
	void SetShininess(const float& shininess) { materialData->shininess = shininess; }
	// DebugModeを有効化
	void DebugMode(bool debugMode) { useWireFrameTexture = debugMode; }
	// SkinClusterのセット(通常使うものではないため気にしないで良い)
	void SetSkinCluster(const std::vector<SkinCluster> skinCluster);
	void CreateSkinningResources(const Skeleton& skeleton);

	// アニメーションの追加
	void AddAnimation(std::string directoryPath, std::string filename, std::string animationName);

	void AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& filenames);


	void SetEnvironmentCoefficient(const float amount) { materialData->environmentCoefficient = amount; }

	void SetEnableMetallic(const bool flag) { materialData->enableMetallic = flag; }

	const bool GetEnableMetallic() const { return materialData->enableMetallic; }

	void SetPBRMaterial(const float metallic, const float roughness);

	// AABBの取得
	const AABB& GetMeshAABB() const { return meshAABB; }
	const std::map<std::wstring, AABB>& GetMultiMeshAABB() const { return multiMeshAABB; }

private:

	std::map<std::string, Animation> animation;
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
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルバッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

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
	// ノード情報のロード
	static Node ReadNode(aiNode* node);
	// アニメーションの読み込み
	static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	// .gltfファイルの読み取り
	static ModelData LoadModelFileGLTF(const std::string& directoryPath, const std::string& fileName);
	// .objファイルの読み取り
	static ModelData LoadModelFileOBJ(const std::string& directoryPath, const std::string& fileName);
	// VertexResourceを作成する
	void CreateVertexResource();
	// MaterialResourceを作成する
	void CreateMaterialResouce();
	// VertexBufferViewを作成する(値を設定するだけ)
	void CreateVertexBufferView();

	// ModelのAABBを作成する
	void CreateAABB();
	

	AABB meshAABB;
	std::map<std::wstring, AABB> multiMeshAABB;

	bool useWireFrameTexture;
	uint32_t whiteTextureIndex;
public:

	Model() = default;
};
