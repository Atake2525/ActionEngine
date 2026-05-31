#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#pragma once


class Camera;

class DebugLineBase
{
private: // シングルトン
    
    DebugLineBase() = default; // デフォルトコンストラクタ
    ~DebugLineBase() = default; // デフォルトデストラクタ
    DebugLineBase(const DebugLineBase&) = delete; // コピーコンストラクタの削除
    DebugLineBase& operator=(const DebugLineBase&) = delete; // コピー代入演算子の削除

    static DebugLineBase* instance;

public:
    /// <summary>
    /// インスタンスの取得
    /// </summary>
    static DebugLineBase* GetInstance();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// シェーダー描画処理
    /// </summary>
    void ShaderDraw();

    // Getter(Camera)
    Camera* GetCamera() const { return camera; }

    // Setter(Camera)
    void SetCamera(Camera* camera) { this->camera = camera; }

    

private:
    Camera* camera = nullptr;

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

