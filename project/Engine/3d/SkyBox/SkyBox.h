#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxcapi.h>
#include "Model.h"


class DirectXBase;
class Camera;

struct SkyBoXData
{
	std::vector<uint32_t> indices;
	std::vector<Vector4> positions;
};

class SkyBox {
private:
	// シングルトンパターンを適用
	static SkyBox* instance;

	// コンストラクタ、デストラクタの隠蔽
	SkyBox() = default;
	~SkyBox() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	SkyBox(SkyBox&) = delete;
	SkyBox& operator=(SkyBox&) = delete;

public:
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static SkyBox* GetInstance();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directxBase);

	void SetTexture(const std::string& filePath);

	void SetCamera(Camera* camera);

	const uint32_t& GetSrvIndex() const { return srvIndex; }

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:

	//void CreateSrvDesc();

	void CreateRootSignature();

	void CreateGraphicsPipeLineState();

private:

	DirectXBase* directxBase_ = nullptr;


	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// Resource作る度に配列を増やしす
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};


	// PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;

	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

private:
	SkyBoXData modelData;
	VertexData* vertexData = nullptr;
	uint32_t* mappedIndex = nullptr;

	// 座標変換リソースのバッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// 座標変換行列リソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrix = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	uint32_t srvIndex;

	// マテリアルのバッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルバッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	Camera* camera_ = nullptr;

	float rotatez = 0.0f;
};

