#include "OffScreenRendering.h"

#include "Logger.h"
#include <cassert>
#include "DirectXBase.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include <algorithm>
#include "ImGuiManager.h"

using namespace Microsoft::WRL;
using namespace Logger;
using namespace std;

OffScreenRendering::OffScreenRendering() {

}

OffScreenRendering::~OffScreenRendering() {

}

void OffScreenRendering::Initialize(DirectXBase& directXBase, SrvManager& srvManager, WinApp& winApp) {
	m_pDirectXBase = &directXBase;
	m_pSrvManager = &srvManager;
	m_pWinApp = &winApp;
	CreateGraphicsPipeLineState();

	renderTextureResource = m_pDirectXBase->CreateRenderTextureResource(m_pWinApp->GetkClientWidth(), m_pWinApp->GetkClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, renderTargetClearValue);

	m_rtvDescriptorHandle = m_pDirectXBase->CreateOffScreenRenderTargetView(renderTextureResource);

	// SRVの設定。FormatはResourceと同じにしておく
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;

	srvIndex = m_pSrvManager->Allocate();

	srvCPUHandle = m_pSrvManager->GetCPUDescriptorHandle(srvIndex);
	srvGPUHandle = m_pSrvManager->GetGPUDescriptorHandle(srvIndex);
	
	DirectX::TexMetadata metadata;
	metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	metadata.mipLevels = 1;

	m_pSrvManager->CreateSRVforTexture2D(srvIndex, renderTextureResource, metadata, D3D12_SRV_DIMENSION_TEXTURE2D);

	grayscaleResouce = m_pDirectXBase->CreateBufferResource(sizeof(Grayscale));
	grayscaleResouce->Map(0, nullptr, reinterpret_cast<void**>(&grayscale));
	grayscale->grayscaleIntensity = 0.0f;
	grayscale->toneColor = { 1.0f, 73.0f / 107.0f, 43.0f / 107.0f };
	grayscale->alpah = 1.0f;

	vignetteResource = m_pDirectXBase->CreateBufferResource(sizeof(Vignette));
	vignetteResource->Map(0, nullptr, reinterpret_cast<void**>(&vignette));
	vignette->enableVignette = false;
	vignette->intensity = 16.0f;
	vignette->scale = 0.8f;

	boxFilterResource = m_pDirectXBase->CreateBufferResource(sizeof(BoxFilter));
	boxFilterResource->Map(0, nullptr, reinterpret_cast<void**>(&boxFilter));
	boxFilter->boxFilterIntensity = 0.0f;
	boxFilter->size = 5;

    dissolveResource = m_pDirectXBase->CreateBufferResource(sizeof(Dissolve));
    dissolveResource->Map(0, nullptr, reinterpret_cast<void**>(&dissolve));
    dissolve->edgeColor = { 1.0f, 1.0f, 1.0f };
    dissolve->threshold = 0.0f;
	noiseSrvIndex = TextureManager::GetInstance()->LoadTexture("Resources/Sprite/noise0.png");
}

void OffScreenRendering::Update() {
#ifndef NDEBUG
	ImGui::SetNextWindowPos(ImVec2{ 0.0f, 18.0f }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2{ 300.0f, float(m_pWinApp->GetkClientHeight()) - 18.0f }, ImGuiCond_FirstUseEver);
	ImGui::Begin("PostEffect");
	/*if (ImGui::IsPopupOpen("PostEffect"))
	{
		Log("openPostEffect");
	}*/
	if (ImGui::CollapsingHeader("Grayscale / グレイスケール", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SliderFloat("グレースケール強度", &grayscale->grayscaleIntensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("ColTone", &grayscale->toneColor.x);
		ImGui::DragFloat("Alpha", &grayscale->alpah);
	}
	if (ImGui::CollapsingHeader("Vignette / ビネット")) {
		ImGui::Checkbox("enable Vignette", &vignette->enableVignette);
		ImGui::DragFloat("intensity", &vignette->intensity, 0.1f);
		ImGui::DragFloat("scale", &vignette->scale, 0.1f);
	}
	if (ImGui::CollapsingHeader("BoxFilter / ボックスフィルター")) {
		ImGui::SliderFloat("enable BoxFilter", &boxFilter->boxFilterIntensity, 0.0f, 1.0f);
		ImGui::SliderInt("size", &boxFilter->size, 1, 50);
	}
	if (ImGui::CollapsingHeader("Dissolve / ディゾルブ")) {
        ImGui::ColorEdit3("EdgeColor", &dissolve->edgeColor.x);
        ImGui::SliderFloat("Threshold", &dissolve->threshold, 0.0f, 1.0f);
	}
	ImGui::End();
#endif _DEBUG


}

void OffScreenRendering::CreateRootSignature() {
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// DescriptorRange
	descriptorRange[0].BaseShaderRegister = 0;                                                   // 0から始まる
	descriptorRange[0].NumDescriptors = 1;                                                       // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	descriptorRange[1].BaseShaderRegister = 1;                                                   // 0から始まる
	descriptorRange[1].NumDescriptors = 1;                                                       // 数は1つ
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;                              // SRVを使う
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

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

	// Resource作る度に配列を増やしす
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;          // VertexShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                              // レジスタ番号0を使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];        // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[2].Descriptor.ShaderRegister = 0;                              // レジスタ番号0とバインド
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[3].Descriptor.ShaderRegister = 1;                    // レジスタ番号1を使う
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[4].Descriptor.ShaderRegister = 2;                    // レジスタ番号2を使う
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[5].Descriptor.ShaderRegister = 3;                    // レジスタ番号3を使う
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[6].DescriptorTable.pDescriptorRanges = &descriptorRange[1];        // Tableの中身の配列を指定
	rootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = m_pDirectXBase->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
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

	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;
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
	vertexShaderBlob = m_pDirectXBase->CompileShader(L"Resources/shaders/PostEffect/Fullscreen.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = m_pDirectXBase->CompileShader(L"Resources/shaders/PostEffect/Dissolve.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = false;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void OffScreenRendering::CreateGraphicsPipeLineState() {
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
	HRESULT hr = m_pDirectXBase->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPilelineState));
	assert(SUCCEEDED(hr));
}

void OffScreenRendering::Draw() {

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	m_pDirectXBase->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	m_pDirectXBase->GetCommandList()->SetPipelineState(graphicsPilelineState.Get());
	m_pDirectXBase->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// grayscale
	m_pDirectXBase->GetCommandList()->SetGraphicsRootConstantBufferView(2, grayscaleResouce->GetGPUVirtualAddress());
	// vignetting
	m_pDirectXBase->GetCommandList()->SetGraphicsRootConstantBufferView(3, vignetteResource->GetGPUVirtualAddress());
	// boxFilter
	m_pDirectXBase->GetCommandList()->SetGraphicsRootConstantBufferView(4, boxFilterResource->GetGPUVirtualAddress());
	// dissolve
	m_pDirectXBase->GetCommandList()->SetGraphicsRootConstantBufferView(5, dissolveResource->GetGPUVirtualAddress());

	m_pSrvManager->SetGraphicsRootDescriptorTable(6, noiseSrvIndex);
	// srvGPUHandleの設定
	m_pSrvManager->SetGraphicsRootDescriptorTable(1, srvIndex);

	// Draw call
	m_pDirectXBase->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void OffScreenRendering::SetGrayscaleIntensity(float value)
{
	float val = clamp(value, 0.0f, 1.0f);
	grayscale->grayscaleIntensity = val;
}
