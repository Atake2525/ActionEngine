#include "DebugLine.h"
#include "Logger.h"
#include <cassert>
#include "DirectXBase.h"
#include "kMath.h"
#include "Camera.h"
#define  NOMINMAX
#include <minmax.h>
#include "ImGuiManager.h"

using namespace Microsoft::WRL;
using namespace Logger;

DebugLine* DebugLine::instance = nullptr;

DebugLine* DebugLine::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new DebugLine;
    }
    return instance;
}

void DebugLine::Finalize()
{
    delete instance;
    instance = nullptr;
}

void DebugLine::Initialize()
{
    CreateGraphicsPipeLineState();

    vertexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(LineVertex) * size_t(MAX_LINE_VERTEX_COUNT));

    // 頂点バッファビューの設定
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    // 頂点データ全体のサイズ
    vertexBufferView.SizeInBytes = sizeof(LineVertex) * MAX_LINE_VERTEX_COUNT;
    // 1頂点あたりのデータサイズ
    vertexBufferView.StrideInBytes = sizeof(LineVertex);
    // 頂点バッファをマップして書き込み可能にする
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&lineMap));

    transformationMatrixResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(LineTransformationMatrix));
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));
    transformationMatrix->WVP = MakeIdentity4x4();

    lineVertices.push_back({ { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } });
    lineVertices.push_back({ { 100.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } });

    transform = Transform::Default;
}

void DebugLine::Update()
{
    ImGui::Begin("Debug Line Editor");

    // --- 新規追加ボタン ---
    if (ImGui::Button("Add New Line")) {
        // デフォルトの線（原点から上に伸びる赤線）を追加
        LineVertex v1, v2;
        v1.position = { 0.0f, 0.0f, 0.0f, 1.0f };
        v1.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤
        v2.position = { 0.0f, 2.0f, 0.0f, 1.0f };
        v2.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤

        lineVertices.push_back(v1);
        lineVertices.push_back(v2);
    }

    ImGui::SameLine();

    // --- 全削除ボタン ---
    if (ImGui::Button("Clear All")) {
        lineVertices.clear();
    }

    ImGui::Separator();

    // --- リスト表示 ---
    // LINELISTなので2つずつ進める
    int removeIndex = -1; // 削除するインデックスを一時保存

    for (size_t i = 0; i < lineVertices.size(); i += 2) {
        // 安全対策: ペアになっていない端数がある場合
        if (i + 1 >= lineVertices.size()) break;

        // IDの衝突を防ぐためにPushIDを使う
        ImGui::PushID((int)i);

        std::string headerName = "lineVertex " + std::to_string(i / 2);

        // ツリー表示で見やすくする
        if (ImGui::TreeNode(headerName.c_str())) {

            // 始点 (Start Point)
            ImGui::Text("Start Point (Index %d)", i);
            // Vector4のxyzだけ編集、wは無視
            ImGui::DragFloat3("Pos##1", &lineVertices[i].position.x, 0.1f);
            ImGui::ColorEdit4("Col##1", &lineVertices[i].color.x);

            // 終点 (End Point)
            ImGui::Text("End Point (Index %d)", i + 1);
            ImGui::DragFloat3("Pos##2", &lineVertices[i + 1].position.x, 0.1f);
            ImGui::ColorEdit4("Col##2", &lineVertices[i + 1].color.x);

            // この線を削除
            if (ImGui::Button("Delete This Line")) {
                removeIndex = (int)i;
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    // --- 削除処理 ---
    // ループ内でeraseするとイテレータが壊れるため、ループ外で行う
    if (removeIndex != -1) {
        // 後ろから削除しないとインデックスがずれるが、今回は特定要素なのでイテレータ計算で削除
        auto it = lineVertices.begin() + removeIndex;
        lineVertices.erase(it, it + 2); // 2要素（始点と終点）を削除
    }

    ImGui::End();

    // ワールド行列の計算
    worldMatrix = MakeAffineMatrix(transform);

    Matrix4x4 viewProjectionMatrix = worldMatrix;

    if (camera)
    {
        viewProjectionMatrix = Multiply(worldMatrix, camera->GetViewProjectionMatrix());
    }

    transformationMatrix->WVP = viewProjectionMatrix;
}

void DebugLine::Draw()
{
    // 2. 頂点データの転送
   // バッファ溢れ防止
    UINT drawVertexCount = min((UINT)lineVertices.size(), MAX_LINE_VERTEX_COUNT);
    memcpy(lineMap, lineVertices.data(), drawVertexCount * sizeof(LineVertex));
    // RootSignatureを設定。PSOに設定しているけど別途設定が必要
    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
    // PSOを設定
    DirectXBase::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
    // 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    DirectXBase::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    // 頂点バッファを設定
    DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    // 定数バッファを設定
    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, transformationMatrixResource->GetGPUVirtualAddress());
    // 描画コマンド
    DirectXBase::GetInstance()->GetCommandList()->DrawInstanced(drawVertexCount, 1, 0, 0);
}

void DebugLine::CreateRootSignature() {
    // RootSignatureの設定
    D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
    // Flagの設定
    descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // IAInputLayoutを有効化
    

    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // ConstantBufferView
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0; // b0

    descriptorRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
    descriptorRootSignature.NumParameters = _countof(rootParameters); // ルートパラメータ数

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    // バイナリを基に生成
    hr = DirectXBase::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    // InputLayoutの設定
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "COLOR";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    // BlendStateの設定
    // すべての色要素を書き込む
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    // 裏面(時計回り)を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    // シェーダーをコンパイルする
    vertexShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/DebugLine/DebugLine.VS.hlsl", L"vs_6_5");
    assert(vertexShaderBlob != nullptr);
    pixelShaderBlob = DirectXBase::GetInstance()->CompileShader(L"Resources/shaders/DebugLine/DebugLine.PS.hlsl", L"ps_6_5");
    assert(pixelShaderBlob != nullptr);
}

void DebugLine::CreateGraphicsPipeLineState()
{
    CreateRootSignature();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するトポロジ(形状)のタイプを設定 Line
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    // どのように画面に色を打ち込むかの設定(気にしなくて良い)
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // 実際に生成
    HRESULT hr = DirectXBase::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
    assert(SUCCEEDED(hr));
}
