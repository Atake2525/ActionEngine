#include "SkyBox.h"
#include "Logger.h"
#include <iostream>
#include <cassert>
#include <DirectXBase.h>
#include "TextureManager.h"
#include "SrvManager.h"
#include "Camera.h"
#include "Light.h"

#ifndef NDEBUG
#include "ImGuiManager.h"
#endif // !NDEBUG


using namespace Logger;

SkyBox* SkyBox::instance = nullptr;

SkyBox* SkyBox::GetInstance() {
	if (instance == nullptr)
	{
		instance = new SkyBox;
	}
	return instance;
}

void SkyBox::Finalize() {
	delete instance;
	instance = nullptr;
}

void SkyBox::SetTexture(const std::string& filePath) {
	srvIndex = TextureManager::GetInstance()->GetSrvIndex(filePath);
}

void SkyBox::SetCamera(Camera* camera) {
	camera_ = camera;
}

void SkyBox::Initialize() {
	CreateGraphicsPipeLineState();

	transformationMatrixResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));

	// 単位行列を書き込んでおく
	transformationMatrix->WVP = MakeIdentity4x4();
	transformationMatrix->World = MakeIdentity4x4();

	materialResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(Material));
	//  書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	//materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->color = { 0.0f, 0.0f, 0.0f, 1.0f };

	// 頂点リソースの作成
	vertexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexData) * 24);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	sunResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(Sun));
	sunResource->Map(0, nullptr, reinterpret_cast<void**>(&sunData));

	sunData->power = 0.0f;
	sunData->topColor = { 0.45f, 0.65f, 1.0f };
	sunData->bottomColor = { 0.02f, 0.05f, 0.15f };
	sunData->sunDirection = { 0.0f, 1.0f, 0.0f };

	// 頂点バッファビューを作成する
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * 24);
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	indexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 36);

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * 36);
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));


	// 右面。描画インデックスは[0, 1, 2][2, 1, 3]で内側を向く
	vertexData[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	mappedIndex[0] = 0;
	mappedIndex[1] = 1;
	mappedIndex[2] = 2;
	mappedIndex[3] = 2;
	mappedIndex[4] = 1;
	mappedIndex[5] = 3;

	// 左面。描画インデックスは[4, 5, 6][6, 5, 7]
	vertexData[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };

	mappedIndex[6] = 4;
	mappedIndex[7] = 5;
	mappedIndex[8] = 6;
	mappedIndex[9] = 6;
	mappedIndex[10] = 5;
	mappedIndex[11] = 7;

	// 前面。描画インデックスは[8, 9, 10][10, 9 , 11]
	vertexData[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };

	mappedIndex[12] = 8;
	mappedIndex[13] = 9;
	mappedIndex[14] = 10;
	mappedIndex[15] = 10;
	mappedIndex[16] = 9;
	mappedIndex[17] = 11;

	// 背面。描画インデックスは[12, 13, 14][14, 13, 15]
	vertexData[12].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[13].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[14].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[15].position = { -1.0f, -1.0f, -1.0f, 1.0f };

	mappedIndex[18] = 12;
	mappedIndex[19] = 13;
	mappedIndex[20] = 14;
	mappedIndex[21] = 14;
	mappedIndex[22] = 13;
	mappedIndex[23] = 15;

	// 上面。描画インデックスは[16, 17, 18][18, 17, 19]
	vertexData[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[17].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData[18].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };

	mappedIndex[24] = 16;
	mappedIndex[25] = 17;
	mappedIndex[26] = 18;
	mappedIndex[27] = 18;
	mappedIndex[28] = 17;
	mappedIndex[29] = 19;

	// 下面。描画インデックスは[20, 21, 22][22, 21, 23]
	vertexData[20].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[21].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	mappedIndex[30] = 20;
	mappedIndex[31] = 21;
	mappedIndex[32] = 22;
	mappedIndex[33] = 22;
	mappedIndex[34] = 21;
	mappedIndex[35] = 23;



}

void SkyBox::CreateRootSignature() {

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
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;                              // レジスタ番号0とバインド
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
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	vertexShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/SkyBox/Skybox.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/SkyBox/Skybox.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void SkyBox::CreateGraphicsPipeLineState() {
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

void SkyBox::Update() {

	Transform transform;
	if (camera_)
	{
		transform = camera_->GetTransform();
		transform.position = camera_->GetWorldPosition();
	}
	else
	{
		transform = { 0.0f };
		transform.scale = { 1.0f, 1.0f, 1.0f };
	}
	// 3DのTransform処理
	//rotatez += SwapRadian(1.0f);
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, Vector3{ 0.0f, 0.0f, 0.0f /*rotatez*/ }, transform.position);

	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	Vector3 sunDirection = Light::GetInstance()->GetDirectionDirectionalLight();
	sunData->sunDirection = { sunDirection.x, -sunDirection.y, sunDirection.z };
	sunData->power = Light::GetInstance()->GetIntensityDirectionalLight();


	transformationMatrix->WVP = worldViewProjectionMatrix;
	transformationMatrix->World = worldMatrix;

#ifndef NDEBUG
	ImGui::Begin("Sun Settings");

	// Power
	ImGui::SliderFloat("Power", &sunData->power, 0.0f, 1.0f);

	// Direction
	ImGui::Text("Sun Direction");
	ImGui::DragFloat3("Direction", &sunData->sunDirection.x, 0.01f);
	// 正規化したい場合
	if (ImGui::Button("Normalize Direction"))
	{
		sunData->sunDirection = Normalize(sunData->sunDirection);
	}

	// Top Color
	ImGui::ColorEdit3("Top Color", &sunData->topColor.x);

	// Bottom Color
	ImGui::ColorEdit3("Bottom Color", &sunData->bottomColor.x);

	ImGui::End();
#endif // !NDEBUG


}

void SkyBox::Draw() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	DirectXBase::GetInstance()->GetCommandList()->SetPipelineState(graphicsPilelineState.Get());
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	DirectXBase::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// wvp用のCBufferの場所を設定
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// wvp用のCBufferの場所を設定
	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, sunResource->GetGPUVirtualAddress());

	DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定

	DirectXBase::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView); // VBVを設定

	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(2, srvIndex);

	DirectXBase::GetInstance()->GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);

}