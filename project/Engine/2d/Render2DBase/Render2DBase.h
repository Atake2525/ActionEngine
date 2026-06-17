#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#pragma once

// スプライト描画用クラス
class Render2DBase {
private:
	// シングルトンパターンを適用
	static Render2DBase* instance;

	// コンストラクタ、デストラクタの隠蔽
	Render2DBase() = default;
	~Render2DBase() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	Render2DBase(Render2DBase&) = delete;
	Render2DBase& operator=(Render2DBase&) = delete;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	// インスタンスの取得
	static Render2DBase* GetInstance();

	// 終了処理
	void Finalize();

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void ShaderDraw();

	const D3D12_VERTEX_BUFFER_VIEW& GetSpriteVertexBufferView() const { return vertexBufferView; }
	const D3D12_INDEX_BUFFER_VIEW& GetSpriteIndexBufferView() const { return indexBufferView; }

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの作成
	void CreateGraphicsPipeLineState();
	void CreateSpriteVertexResource();
	void CreateSpriteIndexResource();

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

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
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
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

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
};
