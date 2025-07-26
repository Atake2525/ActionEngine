#include "OffScreenRnedering.h"

#include "Logger.h"
#include <cassert>
#include "DirectXBase.h"
#include "SrvManager.h"

#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

using namespace Microsoft::WRL;
using namespace Logger;


void OffScreenRnedering::Initialize(DirectXBase* directxBase) {
	directxBase_ = directxBase;
	CreateGraphicsPipeLineState();

	renderTextureResource = directxBase->CreateRenderTextureResource(directxBase_->GetDevice(), WinApp::GetInstance()->GetkClientWidth(), WinApp::GetInstance()->GetkClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, renderTargetClearValue);

	// SRVの設定。FormatはResourceと同じにしておく
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;

	srvIndex = SrvManager::GetInstance()->Allocate();

	srvCPUHandle = SrvManager::GetInstance()->GetCPUDescriptorHandle(srvIndex);
	srvGPUHandle = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);
	// SRVの生成
	//directxBase_->GetDevice()->CreateShaderResourceView(renderTextureResource.Get(), &renderTextureSrvDesc, srvCPUHandle);

	DirectX::TexMetadata metadata;
	metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	metadata.mipLevels = 1;

	SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex, renderTextureResource, metadata, D3D12_SRV_DIMENSION_TEXTURE2D);

	grayscaleResouce = directxBase_->CreateBufferResource(sizeof(Grayscale));
	grayscaleResouce->Map(0, nullptr, reinterpret_cast<void**>(&grayscale));
	grayscale->enableGrayscale = false;
	grayscale->toneColor = { 1.0f, 73.0f / 107.0f, 43.0f / 107.0f };
	grayscale->alpah = 1.0f;

	vignetteResource = directxBase_->CreateBufferResource(sizeof(Vignette));
	vignetteResource->Map(0, nullptr, reinterpret_cast<void**>(&vignette));
	vignette->enableVignette = false;
	vignette->intensity = 16.0f;
	vignette->scale = 0.8f;

	boxFilterResource = directxBase_->CreateBufferResource(sizeof(BoxFilter));
	boxFilterResource->Map(0, nullptr, reinterpret_cast<void**>(&boxFilter));
	boxFilter->enableBoxFilter = false;
	boxFilter->size = 5;

	gaussianFilterResource = directxBase_->CreateBufferResource(sizeof(GaussianFilter));
	gaussianFilterResource->Map(0, nullptr, reinterpret_cast<void**>(&gaussianFilter));
	gaussianFilter->enableGaussianFilter = false;
	gaussianFilter->sigma = 2.0f;
}

void OffScreenRnedering::Update() {
#ifdef _DEBUG
	ImGui::Begin("PostEffect");
	/*if (ImGui::IsPopupOpen("PostEffect"))
	{
		Log("openPostEffect");
	}*/
	ImGui::SetWindowPos(ImVec2{ 0.0f, 18.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) - 18.0f });
	if (ImGui::TreeNode("Grayscale / グレイスケール")) {
		ImGui::Checkbox("有効化", &grayscale->enableGrayscale);
		ImGui::ColorEdit3("ColTone", &grayscale->toneColor.x);
		ImGui::DragFloat("Alpha", &grayscale->alpah);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Vignette / ビネット")) {
		ImGui::Checkbox("有効化", &vignette->enableVignette);
		ImGui::DragFloat("intensity", &vignette->intensity, 0.1f);
		ImGui::DragFloat("scale", &vignette->scale, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("BoxFilter / ボックスフィルター")) {
		ImGui::Checkbox("有効化", &boxFilter->enableBoxFilter);
		ImGui::SliderInt("size", &boxFilter->size, 1, 25);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("GaussianFilter / ガウシアンフィルター")) {
		ImGui::Checkbox("有効化", &gaussianFilter->enableGaussianFilter);
		ImGui::SliderFloat("size", &gaussianFilter->sigma, 1.0f, 10.0f);
		ImGui::TreePop();
	}
	ImGui::End();
#endif _DEBUG


}

void OffScreenRnedering::CreateRootSignature() {
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
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;          // VertexShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                              // レジスタ番号0を使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;        // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[2].Descriptor.ShaderRegister = 0;                              // レジスタ番号0とバインド
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[3].Descriptor.ShaderRegister = 1;                    // レジスタ番号0を使う
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[4].Descriptor.ShaderRegister = 2;                    // レジスタ番号0を使う
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;    // CBVを使う
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	rootParameters[5].Descriptor.ShaderRegister = 3;                    // レジスタ番号0を使う
	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = directxBase_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
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
	vertexShaderBlob = directxBase_->CompileShader(L"Resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = directxBase_->CompileShader(L"Resources/shaders/GaussianFilter.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = false;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void OffScreenRnedering::CreateGraphicsPipeLineState() {
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
	HRESULT hr = directxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPilelineState));
	assert(SUCCEEDED(hr));
}

void OffScreenRnedering::Draw() {

	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directxBase_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	directxBase_->GetCommandList()->SetPipelineState(graphicsPilelineState.Get());
	// grayscale
	directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(2, grayscaleResouce->GetGPUVirtualAddress());
	// vignetting
	directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(3, vignetteResource->GetGPUVirtualAddress());
	// boxFilter
	directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(4, boxFilterResource->GetGPUVirtualAddress());
	// gaussianFilter
	directxBase_->GetCommandList()->SetGraphicsRootConstantBufferView(5, gaussianFilterResource->GetGPUVirtualAddress());
	// srvGPUHandleの設定
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(1, srvIndex);
	//directxBase_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvGPUHandle);
	// Draw call
	directxBase_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}