#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include "Vector4.h"
#include "Vector3.h"
#include <stdint.h>
#pragma once

class DirectXBase;

struct Grayscale
{
	bool enableGrayscale;
	Vector3 toneColor;
	float alpah;
};

struct Vignette
{
	bool enableVignette;
	float intensity; //!< 輝度
	float scale;
};

struct BoxFilter
{
	bool enableBoxFilter;
	int size;
};

struct GaussianFilter
{
	bool enableGaussianFilter;
	float sigma;
};

class OffScreenRnedering
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directxBase);


	void Update();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void Draw();

	DirectXBase* GetDxBase() const { return directxBase_; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetRenderTextureResource() const { return renderTextureResource; }

	const Vector4 GetRenderTargetClearValue() const { return renderTargetClearValue; }

private:
	DirectXBase* directxBase_ = nullptr;

	uint32_t srvIndex;

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
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[6] = {};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle;

	const Vector4 renderTargetClearValue{ 0.0f, 0.3f, 1.0f, 1.0f }; // 分かりやすい赤にする

	/// GraphicsPipeLineState
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;

	Grayscale* grayscale = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> grayscaleResouce;

	Vignette* vignette = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vignetteResource;

	BoxFilter* boxFilter = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> boxFilterResource;

	GaussianFilter* gaussianFilter = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> gaussianFilterResource;
};