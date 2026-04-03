#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include "Culling.h"
//#include "ScanEffect.h"
#pragma once

class Camera;

enum class Object3dDrawMode{
	Normal = 0,
	AwayFront = 1,
	Wireframe = 2,
};

// オブジェクト描画用クラス
class Object3dBase {
private:
	// シングルトンパターンを適用
	static Object3dBase* instance;

	// コンストラクタ、デストラクタの隠蔽
	Object3dBase() = default;
	~Object3dBase() = default;

	// コピーコンストラクタ、コピー代入演算子の封印
	Object3dBase(Object3dBase&) = delete;
	Object3dBase& operator=(Object3dBase&) = delete;

public:
	// インスタンスの取得
	static Object3dBase* GetInstance();

	// 終了処理
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void ShaderDraw(Object3dDrawMode drawMode = Object3dDrawMode::Normal);

	// Getter(Camera)
	Camera* GetDefaultCamera() const { return defaultCamera; }

	// Setter(Camera)
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }

	// Getter(CullingTemplate)
	CullingTemplate GetCullingTemplate() { return cullingTemplateData; }

	void SetCullingTemplateData(const CullingTemplate& data) { cullingTemplateData = data; }


private:

	Camera* defaultCamera = nullptr;

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの作成
	void CreateGraphicsPipeLineState();

public:
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};

private:
	/// Rootsignature
	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[5] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[15] = {};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDescNormal{};
	D3D12_RASTERIZER_DESC rasterizerDescWireFrame{};


	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDescNormal{};
	D3D12_DEPTH_STENCIL_DESC depthStencilDescAwayFront{};

	/// GraphicsPipeLineState
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineStateNormal = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineStateAwayFront = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineStateWireFrame = nullptr;

	CullingTemplate cullingTemplateData;

	

};
