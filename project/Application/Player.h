#include "Object3d.h"
#include <memory>
#pragma once
#include <string>

class Camera;
class Input;

class Player
{
private:
    // プレイヤーステート
    enum class PlayerState : int {
        Idle = 0,
        Move = 1,
        Falling = 2
    };

    // プレイヤーの歩行状態
    enum class PlayerWalkState : int {
        Walk    = 0,
        Run     = 1,
        Crounch = 2
    };

    // コントロールモード
    enum class ControlMode {
        KeyboardMouse,
        Gamepad
    };
    ControlMode m_controlMode = ControlMode::KeyboardMouse;

public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">現在使用しているカメラ</param>
    /// <param name="jsonName">ステージのJsonData</param>
    void Initialize(Camera* camera, const std::string& jsonName);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private: // プレイヤーステート管理
    /// <summary>
    /// 状態の更新
    /// </summary>
    void UpdateState();

    /// <summary>
    /// 入力の処理
    /// </summary>
    void HandleInput();

    /// <summary>
    /// 回転処理
    /// </summary>
    void Rotate();

    /// <summary>
    /// 移動処理
    /// </summary>
    void Move();

    /// <summary>
    /// 重力の適用
    /// </summary>
    void ApplyGravity();

    /// <summary>
    /// カメラのParent設定処理
    /// </summary>
    void UpdateCameraParent();

    // デバッグUIの更新
#ifndef NDEBUG
    void UpdateDebugUI();
    void ToggleGodMode() { m_godMode = !m_godMode; }
    void MovementGodMode();
#endif // !NDEBUG


private: // プレイヤーモデル

    //==================================================
   // タイマー関連
   //==================================================
    float m_timer[5] = { 0.0f };


    //==================================================
    // モデル・カメラ・入力
    //==================================================
    std::unique_ptr<Object3d> m_pModel;
    Camera* m_pCamera = nullptr;
    Input* input;


    //==================================================
    // プレイヤー状態管理
    //==================================================
    PlayerState m_state = PlayerState::Idle;
    PlayerState m_statePre = PlayerState::Idle;

    PlayerWalkState m_walkState = PlayerWalkState::Walk;
    PlayerWalkState m_walkStatePre = PlayerWalkState::Walk;


    AABB m_playerAABB;                      // プレイヤーのAABB当たり判定
    //==================================================
    // プレイヤー移動関連
    //==================================================
    Transform m_transform = Transform::Default;     // 現在の位置・回転・スケール
    Transform m_velocity = Transform::Default;      // 現在の速度
    Vector3 m_gravity = { 0.0f, -9.8f, 0.0f };      // 重力
    Vector2 m_moveInput = Vector2::Zero;            // 入力方向
    float m_maxSpeedTime = 0.3f;                    // 最高速度に到達するまでの時間
    float m_decelTime = 0.2f;                       // 減速時間
    Vector2 m_moveAmount = Vector2::Zero;           // 入力中の移動量
    Vector3 m_moveDirection = Vector3::Zero;        // 移動方向
    float m_turnControlFactor = 1.8f;               // 地上での移動制御係数
    float m_airControlFactor = 5.0f;                // 空中での移動制御係数

    // 移動速度
    float m_moveSpeed = 1.0f;
    const float m_walkSpeed = 5.0f;
    const float m_runSpeed = 10.0f;
    const float m_crounchSpeed = 3.0f;


    //==================================================
    // カメラ関連
    //==================================================
    Transform m_cameraTransform = Transform::Default;
    Transform m_cameraVelocity = Transform::Default;

    float m_fovY = 60.0f;       // 現在のFOV
    float m_afterFovY = 60.0f;  // 変更後のFOV
    float m_fovTimer = 0.0f;    // FOV補間用タイマー
    float m_fovTime = 0.3f;     // FOV補間時間


    //==================================================
    // デバッグ関連（デバッグビルドのみ）
    //==================================================
#ifndef NDEBUG
    bool m_debugMode = false;
    bool m_godMode = false;
#endif // !NDEBUG

};

