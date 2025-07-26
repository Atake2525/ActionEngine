#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <array>

#pragma once

class DirectXBase;
class Camera;

class SkinningObject3dBase {
private:
	// シングルトンパターンを適用
	static SkinningObject3dBase* instance;

	// コンストラクタ、デストラクタの隠蔽
	SkinningObject3dBase() = default;
	~SkinningObject3dBase() = default;

	// コピーコンストラクタ、コピー代入演算子の封印
	SkinningObject3dBase(SkinningObject3dBase&) = delete;
	SkinningObject3dBase& operator=(SkinningObject3dBase&) = delete;

public:
	// インスタンスの取得
	static SkinningObject3dBase* GetInstance();

	// 終了処理
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directxBase);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void ShaderDraw();

	DirectXBase* GetDxBase() const { return directxBase_; }

	// Getter(Camera)
	Camera* GetDefaultCamera() const { return defaultCamera; }

	// Setter(Camera)
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }

private:
	DirectXBase* directxBase_ = nullptr;

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
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[10] = {};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// InputLayout
	//std::array<D3D12_INPUT_ELEMENT_DESC, 5> inputElementDescs{};
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	/// GraphicsPipeLineState
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;
};
