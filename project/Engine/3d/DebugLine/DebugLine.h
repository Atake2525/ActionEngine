#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include <vector>
#pragma once


class Camera;

struct LineVertex
{
    Vector4 position; // xyz:座標、w:不使用
    Vector4 color;    // rgba:色
};

struct LineTransformationMatrix
{
    Matrix4x4 WVP;
};

class DebugLine
{
private: // シングルトン
    
    DebugLine() = default; // デフォルトコンストラクタ
    ~DebugLine() = default; // デフォルトデストラクタ
    DebugLine(const DebugLine&) = delete; // コピーコンストラクタの削除
    DebugLine& operator=(const DebugLine&) = delete; // コピー代入演算子の削除

    static DebugLine* instance;

public:
    /// <summary>
    /// インスタンスの取得
    /// </summary>
    static DebugLine* GetInstance();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// シェーダー描画処理
    /// </summary>
    void Draw();

    // Getter(Camera)
    Camera* GetCamera() const { return camera; }

    // Setter(Camera)
    void SetCamera(Camera* camera) { this->camera = camera; }

private:
    Camera* camera = nullptr;

    UINT MAX_LINE_VERTEX_COUNT = 2048;

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

    LineVertex* lineMap;
    std::vector<LineVertex> lineVertices;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;

    LineTransformationMatrix* transformationMatrix = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>transformationMatrixResource = nullptr;

    Transform transform;
    Matrix4x4 worldMatrix;

};

