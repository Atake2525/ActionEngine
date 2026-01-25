#include "DebugLine.h"
#include "DirectXBase.h"
#include "DebugLineBase.h"
#include "kMath.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include <algorithm>
#include <cmath>

#ifndef XM_PI
#define XM_PI 3.141592654f
#endif

void DebugLine::Initialize() {
    vertexResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(LineVertex) * size_t(MAX_LINE_VERTEX_COUNT));
    vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(LineVertex) * MAX_LINE_VERTEX_COUNT;
    vertexBufferView.StrideInBytes = sizeof(LineVertex);
    vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&lineMap));

    transformationMatrixResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(LineTransformationMatrix));
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));
    transformationMatrix->WVP = MakeIdentity4x4();

    transform = Transform::Default;
    camera = DebugLineBase::GetInstance()->GetCamera();
}

// --- ヘルパー: 線を追加（ローカル座標） ---
void DebugLine::AddLine(const Vector3& p1, const Vector3& p2, const Vector4& color) {
    baseVertices.push_back({ {p1.x, p1.y, p1.z, 1.0f}, color });
    baseVertices.push_back({ {p2.x, p2.y, p2.z, 1.0f}, color });
}

// --- ヘルパー: 円を描画 ---
void DebugLine::AddCircle(const Vector3& center, float radius, const Vector3& normal, const Vector4& color, int segments) {
    // normalが(0,1,0)ならXZ平面、(1,0,0)ならYZ平面、(0,0,1)ならXY平面
    float angleStep = (XM_PI * 2.0f) / segments;

    auto GetOffset = [&](float angle) -> Vector3 {
        float s = sinf(angle) * radius;
        float c = cosf(angle) * radius;
        if (fabsf(normal.y) > 0.5f) return { c, 0.0f, s };      // XZ平面
        if (fabsf(normal.x) > 0.5f) return { 0.0f, c, s };      // YZ平面
        return { c, s, 0.0f };                                   // XY平面
        };

    for (int i = 0; i < segments; ++i) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;
        Vector3 off1 = GetOffset(a1);
        Vector3 off2 = GetOffset(a2);
        Vector3 p1 = { center.x + off1.x, center.y + off1.y, center.z + off1.z };
        Vector3 p2 = { center.x + off2.x, center.y + off2.y, center.z + off2.z };
        AddLine(p1, p2, color);
    }
}

// --- AABB (軸平行境界ボックス) ---
void DebugLine::AddBox(const Vector3& size, const Vector4& color) {
    Vector3 half = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
    Vector3 p[8] = {
        { -half.x, -half.y, -half.z }, { +half.x, -half.y, -half.z },
        { +half.x, +half.y, -half.z }, { -half.x, +half.y, -half.z },
        { -half.x, -half.y, +half.z }, { +half.x, -half.y, +half.z },
        { +half.x, +half.y, +half.z }, { -half.x, +half.y, +half.z }
    };

    // 12本のエッジ
    AddLine(p[0], p[1], color); AddLine(p[1], p[2], color);
    AddLine(p[2], p[3], color); AddLine(p[3], p[0], color);
    AddLine(p[4], p[5], color); AddLine(p[5], p[6], color);
    AddLine(p[6], p[7], color); AddLine(p[7], p[4], color);
    AddLine(p[0], p[4], color); AddLine(p[1], p[5], color);
    AddLine(p[2], p[6], color); AddLine(p[3], p[7], color);
}

// --- OBB (回転を伴う境界ボックス) ---
// 実装上はAddBoxと同じだが、transform.rotateが効くため回転ボックスになる
void DebugLine::AddOBB(const Vector3& size, const Vector4& color) {
    AddBox(size, color);
}

// --- 球 ---
void DebugLine::AddSphere(float radius, const Vector4& color, int segments) {
    Vector3 origin = { 0, 0, 0 };
    AddCircle(origin, radius, { 0,1,0 }, color, segments); // XZ平面
    AddCircle(origin, radius, { 1,0,0 }, color, segments); // YZ平面
    AddCircle(origin, radius, { 0,0,1 }, color, segments); // XY平面
}

// --- カプセル (Y軸方向) ---
void DebugLine::AddCapsule(float height, float radius, const Vector4& color, int segments) {
    float halfHeight = height * 0.5f;
    float cylinderHalf = std::max(0.0f, halfHeight - radius);

    Vector3 top = { 0, cylinderHalf, 0 };
    Vector3 bottom = { 0, -cylinderHalf, 0 };

    // 円筒部分
    AddCircle(top, radius, { 0,1,0 }, color, segments);
    AddCircle(bottom, radius, { 0,1,0 }, color, segments);

    // 縦の線（4本）
    AddLine({ radius, top.y, 0 }, { radius, bottom.y, 0 }, color);
    AddLine({ -radius, top.y, 0 }, { -radius, bottom.y, 0 }, color);
    AddLine({ 0, top.y, radius }, { 0, bottom.y, radius }, color);
    AddLine({ 0, top.y, -radius }, { 0, bottom.y, -radius }, color);

    // 半球（簡易的に半円を4方向）
    float angleStep = XM_PI / (segments / 2);
    for (int i = 0; i < segments / 2; ++i) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;
        float s1 = sinf(a1) * radius, c1 = cosf(a1) * radius;
        float s2 = sinf(a2) * radius, c2 = cosf(a2) * radius;

        // 上半球
        AddLine({ c1, top.y + s1, 0 }, { c2, top.y + s2, 0 }, color);
        AddLine({ 0, top.y + s1, c1 }, { 0, top.y + s2, c2 }, color);

        // 下半球
        AddLine({ c1, bottom.y - s1, 0 }, { c2, bottom.y - s2, 0 }, color);
        AddLine({ 0, bottom.y - s1, c1 }, { 0, bottom.y - s2, c2 }, color);
    }
}

void DebugLine::Update() {
    ImGui::Begin("Debug Collision Visualizer");

    // --- Transform設定 ---
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Position", &transform.translate.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
    }

    // --- プリミティブ追加 ---
    static float color[4] = { 1.0f, 1.0f, 0.0f, 1.0f }; // デフォルトは黄色
    ImGui::ColorEdit4("Primitive Color", color);
    Vector4 col = { color[0], color[1], color[2], color[3] };

    if (ImGui::CollapsingHeader("Add Primitives", ImGuiTreeNodeFlags_DefaultOpen)) {

        // AABB
        static float boxSize[3] = { 2.0f, 2.0f, 2.0f };
        ImGui::DragFloat3("Box Size", boxSize, 0.1f);
        if (ImGui::Button("Add AABB")) {
            AddBox({ boxSize[0], boxSize[1], boxSize[2] }, col);
        }

        ImGui::Separator();

        // OBB (rotationを使う)
        if (ImGui::Button("Add OBB (uses rotation)")) {
            AddOBB({ boxSize[0], boxSize[1], boxSize[2] }, col);
        }

        ImGui::Separator();

        // Sphere
        static float sphereRadius = 1.0f;
        ImGui::DragFloat("Sphere Radius", &sphereRadius, 0.1f);
        if (ImGui::Button("Add Sphere")) {
            AddSphere(sphereRadius, col);
        }

        ImGui::Separator();

        // Capsule
        static float capsuleHeight = 3.0f;
        static float capsuleRadius = 0.5f;
        ImGui::DragFloat("Capsule Height", &capsuleHeight, 0.1f);
        ImGui::DragFloat("Capsule Radius", &capsuleRadius, 0.1f);
        if (ImGui::Button("Add Capsule")) {
            AddCapsule(capsuleHeight, capsuleRadius, col);
        }
    }

    // --- クリア ---
    if (ImGui::Button("Clear All Shapes")) {
        baseVertices.clear();
    }

    ImGui::Text("Vertices: %d", (int)baseVertices.size());
    ImGui::End();

    // --- Transform適用 ---
    // baseVerticesに対してワールド行列を適用してlineVerticesを生成
    lineVertices.clear();
    Matrix4x4 worldMat = MakeAffineMatrix(transform);

    for (const auto& v : baseVertices) {
        Vector3 pos = { v.position.x + transform.translate.x, v.position.y + transform.translate.y, v.position.z + transform.translate.z };
        Vector3 transformed = TransformNormal(pos, worldMat); // あなたのkMathにある変換関数
        lineVertices.push_back({ {transformed.x, transformed.y, transformed.z, 1.0f}, v.color });
    }

    // ワールド行列の計算（カメラとの合成）
    Matrix4x4 viewProjectionMatrix = MakeIdentity4x4();
    if (camera) {
        viewProjectionMatrix = (camera->GetViewProjectionMatrix());
    }
    transformationMatrix->WVP = viewProjectionMatrix;

    // 頂点データの転送
    drawVertexCount = std::min((UINT)lineVertices.size(), MAX_LINE_VERTEX_COUNT);
    memcpy(lineMap, lineVertices.data(), drawVertexCount * sizeof(LineVertex));
}

void DebugLine::Draw() {
    DirectXBase::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, transformationMatrixResource->GetGPUVirtualAddress());
    DirectXBase::GetInstance()->GetCommandList()->DrawInstanced(drawVertexCount, 1, 0, 0);
}
