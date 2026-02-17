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
        Walk     = 0,
        Run      = 1,
        Crounch  = 2,
        WallDash = 3,
        Sliding  = 4,
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
    /// パルクール処理の更新(ステート)
    /// </summary>
    void UpdateParkourState();

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
    /// 歩行処理
    /// </summary>
    void Walk();

    /// <summary>
    /// ウォールラン処理
    /// </summary>
    void WallRun();

    /// <summary>
    /// ジャンプ処理
    /// </summary>
    void Jump();

    /// <summary>
    /// 衝突判定の適用
    /// </summary>
    void ApplyCollision();

    /// <summary>
    /// 重力の適用
    /// </summary>
    void ApplyGravity();

    /// <summary>
    /// カメラのParent設定処理
    /// </summary>
    void UpdateCameraParent();

    /// <summary>
    /// カメラ効果の適用
    /// </summary>
    void ApplyCameraEffect();

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
    float m_delta;

    float m_moveSpeedTimer = 0.0f;    // 移動速度系タイマー
    float m_speedDecelTime = 0.2f;     // 減速時間
    float m_maxSpeedTime = 0.3f;                    // 加速時間
    // 減速フラグ
    bool m_isSpeedDecel = false;
    float m_speedBefore = 0.0f; // 減速前の速度
    float m_speedAfter = 0.0f;  // 減速後の速度

    //==================================================
    // モデル・カメラ・入力
    //==================================================
    std::unique_ptr<Object3d> m_pModel;
    Camera* m_pCamera = nullptr;
    Input* input;


    //==================================================
    // プレイヤー状態管理
    //==================================================
    bool m_isFirstInput = false;
    PlayerState m_state = PlayerState::Idle;
    PlayerState m_statePre = PlayerState::Idle;

    PlayerWalkState m_walkState = PlayerWalkState::Walk;
    PlayerWalkState m_walkStatePre = PlayerWalkState::Walk;

    //==================================================
    // プレイヤー状態管理(パルクール)
    //==================================================
    bool m_onGround = false;

    //==================================================
    // プレイヤー移動関連
    //==================================================
    AABB m_playerAABB;                      // プレイヤーのAABB当たり判定
    Transform m_firstTransform = Transform::Default;    // 初期位置・回転・スケール
    Transform m_transform = Transform::Default;     // 現在の位置・回転・スケール
    Transform m_velocity = Transform::Default;      // 現在の速度
    Vector3 m_gravity = { 0.0f, -1.0f, 0.0f };      // 重力
    Vector3 m_moveDirection = Vector3::Zero;        // 移動方向
    Vector2 m_moveInput = Vector2::Zero;            // 入力方向
    Vector2 m_moveAmount = Vector2::Zero;           // 入力中の移動量
    float m_jumpInput = 0.0f;                          // ジャンプ入力
    float m_fallVelocity = 0.0f;                    // 落下速度
    float m_fallVelocityMax = -1.6f;                // 落下速度上限
    float m_decelTime = 0.2f;                       // 減速時間
    float m_turnControlFactor = 1.8f;               // 地上での移動制御係数
    //float m_airControlFactor = 5.0f;                // 空中での移動制御係数
    float m_airControlFactor = 1.8f;                // 空中での移動制御係数

    //================
    // ウォールラン関連
    //================
    bool m_wallRunning = false;
    float m_wallRunGravity = -0.3f;
    Vector2 m_wallRunStartInput = Vector2::Zero;
    Vector3 m_wallRunStartRotate = Vector3::Zero;
    Vector3 m_wallRunDirection = Vector3::Zero;
    Vector3 m_wallPenetration = Vector3::Zero;
    bool m_isStartWallRun = false;
    float m_wallRunTimer = 0.0f;
    float m_wallRunTime = 0.2f;

    // 移動速度
    float m_moveSpeed = 1.0f;
    float m_decelMoveSpeed = 1.0f;
    float m_moveSpeedPre = 1.0f;
    float m_walkSpeed = 8.0f;
    float m_runSpeed = 14.0f;
    float m_crounchSpeed = 5.0f;
    float m_playerSpeed = 0.0f; // 現在の速度
    float m_jumpForce = 0.045f; // ジャンプ力

    //==================================================
    // カメラ関連
    //==================================================
    Transform m_cameraTransform = Transform::Default;
    Transform m_cameraVelocity = Transform::Default;

    // Fov補間用タイマー
    bool m_isFovChange = false;
    float m_fovBefore = 0.0f;
    float m_fovAfter = 0.0f;
    float m_fov = 0.0f;
    float m_fovPre = 0.0f;
    float m_fovChangeTimer = 0.0f;    // FOV補間用タイマー
    float m_fovChangeTime = 0.1f;     // FOV補間時間
    float m_fovDefault = 1.0f; // デフォルトFOV
    float m_fovRun = 1.3f;    // ダッシュ時FOV


    //==================================================
    // デバッグ関連（デバッグビルドのみ）
    //==================================================
#ifndef NDEBUG
    bool m_debugMode = false;
    bool m_godMode = false;
#endif // !NDEBUG

};

