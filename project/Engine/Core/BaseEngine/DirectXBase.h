#pragma once
#include "WinApp.h"
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <array>
#include <dxcapi.h>
#include <string>
#include <cstdint>
#include <chrono>
#include <thread>
#include "externels/DirectXTex/DirectXTex.h"
#include "externels/DirectXTex/d3dx12.h"
#include "Vector4.h"
#include "Vector3.h"
#include "OffScreenRnedering.h"

class DirectXBase {
public:

	void Update();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void InitializePosteffect();

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// 描画前処理
	void PreDrawRenderTexture();

	// 描画後処理
	void PostDrawRenderTexture();


	// 終了処理
	void Finalize();

	const DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }

	const D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return rtvDesc; }

	// DSVとRTVも作る

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	// getter
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device.Get(); }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }
	// SwapChainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> GetSwapChainResources(UINT i) const { return swapChainResources[i].Get(); };

	/// <summary>
	/// ディスクリプターヒープの作成関数
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// DirectX12のTextureResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// RenderTextureの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height, DXGI_FORMAT format, const Vector4& clearColor);

	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

private:


	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	// FPS固定初期化
	void InitializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();

	// 起動時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	/// <summary>
	/// デバイス初期化
	/// </summary>
	void InitializeDevice();
	/// <summary>
	/// スワップチェイン初期化
	/// </summary>
	void CreateSwapChain();
	/// <summary>
	/// 深度バッファ初期化
	/// </summary>
	void CreateDepthBuffer();
	/// <summary>
	/// コマンド関連の初期化
	/// </summary>
	void InitializeCommands();
	/// <summary>
	/// 各種ディスクリプターヒープの作成
	/// </summary>
	void MakeDescriptorHeap();
	/// <summary>
	/// レンダーターゲットビューの初期化
	/// </summary>
	void InitializeRenderTargetView();
	/// <summary>
	/// 深度ステンシルビューの初期化
	/// </summary>
	void InitializeDepthStenCilView();
	/// <summary>
	/// フェンスの初期化
	/// </summary>
	void InitializeFence();
	/// <summary>
	///  ビューポート矩形の初期化
	/// </summary>
	void InitializeViewPortRect();
	/// <summary>
	/// シザー矩形の初期化
	/// </summary>
	void InitializeScissorRect();
	/// <summary>
	/// DXCコンパイラの生成
	/// </summary>
	void CreateDXCCompiler();
	/// <summary>
	/// ImGuiの初期化
	/// </summary>
	void InitializeImgui();
	/// <summary>
	/// 指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	//Microsoft::WRL::ComPtr<ID3D12Resource>CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);

	// ポインタ
	WinApp* winApp_ = nullptr;

	HRESULT hr;
	// DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// スワップチェイン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// 深度バッファ
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// レンダーターゲットビュー
	// スワップチェイン
	// SwapChainからResourceを引っ張ってくる
	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	D3D12_CPU_DESCRIPTOR_HANDLE rtvTextureHandle;

	//Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource;

	//const Vector4 renderTargetClearValue{ 1.0f, 0.0f, 0.0f, 1.0f }; // 分かりやすい赤にする

	/*D3D12_CPU_DESCRIPTOR_HANDLE srvCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle;*/


	//// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
	// ビューポート矩形
	D3D12_VIEWPORT viewPort{};
	// シザー矩形
	D3D12_RECT scissorRect{};
	// dxcCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//// DepthStencilStateの設定

	//
	//// バックバッファの番号
	UINT backBufferIndex;
	// リソースバリア
	D3D12_RESOURCE_BARRIER barrier{};
	// dsvHandle
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	//


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	// RTV様のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

	//// SRV様のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、shaderVisibleはtrue
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはFalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	// offScreenRendering用の3角形の変数宣言
	void CreatePSO();

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};

	/// Rootsignature
	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
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
	D3D12_DEPTH_STENCIL_DESC depthStencilDescPso{};

	/// GraphicsPipeLineState
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;

	uint32_t textureIndex;

	OffScreenRnedering* offscreen = nullptr;

	float clearColor[4];

	//struct Monotone
	//{
	//	float x, y, z;
	//};

	////Monotone monotone = { 107.0f, 74.0f, 43.0f };
	//Monotone monotone;

	//Microsoft::WRL::ComPtr<ID3D12Resource> monotoneResouce;
};