#include "Vector4.h"
#include "Vector3.h"
#include <wrl.h>
#include <d3d12.h>
#include "Matrix4x4.h"
#include "Transform.h"
#include <vector>

#pragma once

class Camera;

struct LineVertex {
    Vector4 position; // xyz:座標、w:不使用
    Vector4 color;    // rgba:色
};

struct LineTransformationMatrix {
    Matrix4x4 WVP;
};

class DebugLineBase;

class DebugLine {
public:
    void Initialize(DebugLineBase& debugLineBase);
    void Update();
    void Draw();

    // --- 追加: 図形生成関数 ---
    // ローカル座標（原点中心）で図形を生成してlineVerticesに追加
    void AddBox(const Vector3& size, const Vector4& color);
    void AddSphere(float radius, const Vector4& color, int segments = 16);
    void AddCapsule(float height, float radius, const Vector4& color, int segments = 16);
    void AddOBB(const Vector3& size, const Vector4& color); // 実体はAddBoxと同じだが、名前で区別

    void SetTransform(const Transform& transform) { this->transform = transform; }

private:
    DebugLineBase* m_pDebugLineBase;
    Camera* m_pCamera;
    UINT drawVertexCount;
    UINT MAX_LINE_VERTEX_COUNT = 1024;

    LineVertex* lineMap;
    std::vector<LineVertex> lineVertices;

    // 元の形状データ（Transform適用前）を保持
    std::vector<LineVertex> baseVertices;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;

    LineTransformationMatrix* transformationMatrix = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;

    Transform transform;
    Matrix4x4 worldMatrix;

    // --- 内部ヘルパー ---
    void AddLine(const Vector3& p1, const Vector3& p2, const Vector4& color);
    void AddCircle(const Vector3& center, float radius, const Vector3& normal, const Vector4& color, int segments);
};
