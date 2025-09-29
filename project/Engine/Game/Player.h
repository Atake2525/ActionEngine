#include "Camera.h"
#include "Input.h"
#include "Object3d.h"

#pragma once

// プレイヤーの行動ステータス
enum class PlayerMoveType {
    Idle,
    Crouch,
    Walk,
    Backwalk,
    Sneak,
    Dash,
    Jump,
};

class Player
{
public:

    ~Player();

    void Initialize(Camera* camera, Input* input, const Transform startPoint, const bool DebugMode = false);

    void Update();

    void Draw();

    void SetClearDistance(const float clearDistance) { clearDistance_ = clearDistance; }

    const bool IsClear() const;

private: // メンバ変数宣言

    bool debugMode_ = false;
    bool cameraMove_ = true;
    bool parent_ = true;
    Camera* camera = nullptr;
    Input* input = nullptr;

    Object3d* playerModel_ = nullptr;

    AABB playerAABB_;

private: // ステータス(移動系)宣言
    Transform playerTransform_;
    
    Vector3 moveVelocity_;

    float translateSpeed_ = 0.0f; // 

private: // ステータス(カメラ系)宣言
    Transform cameraTransform;
    Vector2 cameraSpeed = { 0.3f, 0.3f };
    Matrix4x4 cameraMatrix;

    Vector3 cameraOffset_ = { 0.0f, -0.08f, 0.0f };

    float fovTime_ = 0.0f;
    float fovY_ = 0.45f; // 現在のFov数値
    float afterFovY_ = 0.0f; // 変更後のFov数値

    float fovChangeSpeed_ = 0.2f;

    float normalFovY_ = 1.0f; // ダッシュ中以外のFov数値
    float fovYBoost_ = 0.3f; // ダッシュ中のFovの上昇値(normalFovY_を参照)

private: // 中心からクリアまでの距離

    float clearDistance_ = 1000.0f;

private: // ステータス関係の関数宣言

    void Rotation();

    void Move();

    void Sneak();

private:
    void DebugUpdate();
};

