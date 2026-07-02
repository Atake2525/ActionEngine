#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#pragma once


class Camera;
class DirectXBase;

class DebugLineBase
{
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(DirectXBase& directXBase);

    /// <summary>
    /// シェーダー描画処理
    /// </summary>
    void ShaderDraw();

    // Getter(Camera)
    Camera* GetCamera() const { return m_pCamera; }

    // Setter(Camera)
    void SetCamera(Camera* camera) { this->m_pCamera = camera; }

    DirectXBase& GetDirectXBase() { return *m_pDirectXBase; }
    

private:
    DirectXBase* m_pDirectXBase;
    Camera* m_pCamera = nullptr;

    // ルートシグネチャの作成
    void CreateRootSignature();
    // グラフィックスパイプラインの作成
    void CreateGraphicsPipeLineState();

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    D3D12_BLEND_DESC blendDesc{};
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

};

