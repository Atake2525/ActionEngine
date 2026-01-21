#include "Object3d.h"
#include <memory>
#pragma once
#include <string>

class Camera;

class Player
{
private:
    enum class PlayerState {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling
    };

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

    // タイマー
    float m_timer[5] = { 0.0f };

    std::unique_ptr<Object3d> m_pModel;
    Camera* m_pCamera = nullptr;

    PlayerState m_state = PlayerState::Idle;
    PlayerState m_statePre = PlayerState::Idle;

    // プレイヤーの移動関連
    Transform m_transform = Transform::Default;
    Transform m_velocity = Transform::Default;
    Vector3 m_gravity = { 0.0f, -9.8f, 0.0f };
    Vector2 m_moveInput = Vector2::Zero;

    // カメラ関連
    Transform m_cameraTransform = Transform::Default;
    Transform m_cameraVelocity = Transform::Default;
    float m_fovY = 60.0f;
    float m_afterFovY = 60.0f;
    float m_fovTimer = 0.0f;
    float m_fovTime = 0.3f;

#ifndef NDEBUG
    // デバッグ用の変数
    bool m_godMode = false;

#endif // !NDEBUG


};

