#include "Render2DBase.h"
#include "Logger.h"
#include <cassert>
#include <cstdint>
#include "DirectXBase.h"

using namespace Microsoft::WRL;
using namespace Logger;

Render2DBase* Render2DBase::instance = nullptr;

Render2DBase* Render2DBase::GetInstance() {
	if (instance == nullptr) {
		instance = new Render2DBase;
	}
	return instance;
}

void Render2DBase::Finalize() {
	delete instance;
	instance = nullptr;
}

void Render2DBase::Initialize() {
	CreateSpriteVertexResource();
	CreateSpriteIndexResource();
	CreateGraphicsPipeLineState();
}

void Render2DBase::CreateSpriteVertexResource() {
	vertexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexData) * 4);

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	vertexData[0].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData[0].texcoord = { 0.0f, 0.0f };
	vertexData[0].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[1].position = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertexData[1].texcoord = { 1.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f, 1.0f };

	vertexData[3].position = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertexData[3].texcoord = { 0.0f, 1.0f };
	vertexData[3].normal = { 0.0f, 0.0f, 1.0f };

	vertexResource->Unmap(0, nullptr);
}

void Render2DBase::CreateSpriteIndexResource() {
	indexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 3;
	indexData[3] = 3;
	indexData[4] = 1;
	indexData[5] = 2;

	indexResource->Unmap(0, nullptr);
}

void Render2DBase::CreateRootSignature() {
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	descriptorRange[0].BaseShaderRegister = 0;                                                   // 0から始まる
	descriptorRange[0].NumDescriptors = 1;                                                       // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

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

	// RootParameter作成。複数設定できるので配列。今回は結果1つだけの長さ1の配列
	// D3D12_ROOT_PARAMETER rootParameter[1] = {};

	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                              // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;          // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;                              // レジスタ番号0を使う
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;        // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = DirectXBase::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
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
	vertexShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/Sprite/Sprite.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/Sprite/Sprite.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = false;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void Render2DBase::CreateGraphicsPipeLineState() {
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
	HRESULT hr = DirectXBase::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPilelineState));
	assert(SUCCEEDED(hr));
}

void Render2DBase::ShaderDraw() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	DirectXBase::GetInstance()->GetCommandList()->SetPipelineState(graphicsPilelineState.Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	DirectXBase::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
