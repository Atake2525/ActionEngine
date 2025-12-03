#include "Transform.h"
#include <memory>
#include "Object3d.h"
#include "Input.h"


#pragma once

class Camera;
class Input;

/// <summary>
/// プレイヤー
/// </summary>
class ActionPlayer {
public:
    // コンストラクタ
    ActionPlayer();
    // デストラクタ
    ~ActionPlayer();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="camera">Objectに適用しているカメラのポインタ</param>
    void Initialize(Camera* camera);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

private: // メンバ変数

    Input* m_pInput;
    Camera* m_pCamera;
    std::unique_ptr<Object3d> m_pPlayerModel;
    AABB m_playerAABB;
    Transform m_playerTransform;

///===== 入力・移動処理 =====///
    Vector3 m_velocity;       // 現在の移動速度
    float m_accelTime;        // 加速時間
    float m_moveSpeed;        // 移動速度保管用
    float m_walkSpeed;        // 通常移動速度
    float m_dashSpeed;        // ダッシュ速度
    float m_crouchSpeed;      // しゃがみ時の移動速度
    float m_jumpForce;        // ジャンプ力
    float m_wallJumpForce;    // 壁ジャンプ力
    float m_gravity;          // 重力加速度
    Vector2 m_inputDirection; // WASD入力方向

///===== プレイヤー状態 =====///
    bool m_isGround;      // 地面に接しているか
    bool m_isDash;        // ダッシュ状態か
    bool m_isCrouch;      // しゃがみ状態か
    bool m_isWallRun;     // 壁走り中か
    bool m_isWallJump;    // 壁ジャンプ中か
    Vector3 m_wallNormal; // 壁の法線

///===== 環境判定 =====///
    float m_groundDistance; // 地面との距離判定
    float m_wallDistance;   // 壁との距離判定


/// ===== 演出 =====///
    float m_walkFov;
    float m_dashFov;
    float m_fovChangeTime;

private: // 関数
///===== 入力・移動処理 =====///

    void HandleInput();            // WASDやジャンプなどの入力処理
    void Move();                   // 移動処理 (地上・空中)
    void ApplyGravity();           // 重力の適用
    void Jump();                   // ジャンプ処理
    void Crouch();   // しゃがみ切り替え

///===== 壁アクション =====///

    bool CheckWall();    // 壁との接触判定
    void StartWallRun(); // 壁走り開始処理
    void StopWallRun();  // 壁走り終了処理
    void WallJump();     // 壁ジャンプ処理

///===== 状態処理 =====///

    void CheckGround();  // 地面判定
    void UpdateStates(); // 各種状態の更新

///===== カメラ制御 =====///

    void HandleMouseLock(); // マウスによる視点操作

///===== 当たり判定の適用 =====///

    void ApplyCollision(); // 当たり判定の適用

///===== 演出系更新 =====///

    void UpdateEffects();

///===== デバッグ用関数 =====//

    void Debug();
};

