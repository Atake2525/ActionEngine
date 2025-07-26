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

private: // メンバ変数宣言

    bool debugMode_ = false;
    bool cameraMove_ = true;
    bool parent_ = true;
    Camera* camera = nullptr;
    Input* input = nullptr;

    Object3d* playerModel_ = nullptr;


private: // ステータス(移動系)宣言
    Transform playerTransform_;
    PlayerMoveType moveType_ = PlayerMoveType::Idle;
    PlayerMoveType moveTypePre_ = PlayerMoveType::Idle;

    Vector3 moveVelocity_;
    Vector3 speed_ = { 0.0f, 0.0f, 0.0f }; // 移動速度
    float speedLimit_ = 2.5f; // 移動速度限界

    float translateAcceleration_ = 0.15f; // 慣性(接地状態)
    float flyAcceleration_ = 0.04f; // 慣性(ジャンプ中)

    // 各アニメーションの速度倍率
    float walkSpeed_ = 0.031f; 
    float backwalkSpeed_ = 0.031f;
    float sneakSpeed_ = 0.032f;
    float dashSpeed_ = 0.1f;

    float easingTime = 0.0f;

    bool jump_ = false;

    float jumpAcceleration_ = 1.6f; // ジャンプの移動量
    float fallLimit_ = -3.4f; // 落下速度上限

    float fallAcceleration_ = 0.102f; // 落下の加速度

private: // ステータス(カメラ系)宣言
    Transform cameraTransform;
    Vector2 cameraSpeed = { 0.3f, 0.3f };
    Matrix4x4 cameraMatrix;

    Vector3 cameraOffset_ = { 0.0f, -0.08f, 0.4f };

    float fovTime_ = 0.0f;
    float fovY_ = 0.45f; // 現在のFov数値
    float afterFovY_ = 0.0f; // 変更後のFov数値

    float fovChangeSpeed_ = 0.2f;

    float normalFovY_ = 0.55f; // ダッシュ中以外のFov数値
    float dashFovY_ = 0.75f; // ダッシュ中のFov数値

private: // ステータス関係の関数宣言

    void Rotation();

    void Move();
    
    void Sneak();
     
private:
    void DebugUpdate();
};

