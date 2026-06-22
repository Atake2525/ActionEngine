#include "Object3d.h"
#include <memory>
#pragma once
#include <string>
#include "PlayerState.h"

class Camera;
class Input;

struct PlayerCommand {
    Vector2 move = Vector2::Zero;
    Vector3 eye = Vector3::Zero;
    bool crouch = false;
    bool run = false;
    bool jump = false;
};

class Player
{
private:
    // プレイヤーステート
    enum class WalkState : int {
        Idle = 0,
        Move = 1,
        Falling = 2
    };

    // プレイヤーの歩行状態
    enum class PlayerWalkState : int {
        Walk = 0,
        Run = 1,
        Crouch = 2,
        WallRun = 3,
        Sliding = 4,
        Climbing = 5,
    };

    // コントロールモード
    enum class ControlMode {
        KeyboardMouse,
        Gamepad
    };
    ControlMode m_controlMode = ControlMode::KeyboardMouse;

public:

    ~Player();

    const Capsule GetCapsule() { return m_pModel->GetCapsule(); }
    const AABB& GetAABB() const { return m_playerAABB; }

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
    void ChangeState(std::unique_ptr<PlayerState> nextState);

    void UpdateModel();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// プレイヤーの操作を無効にする
    /// </summary>
    void SetFreeze(const bool& isFreeze) { m_IsFreeze = isFreeze; };

    // よじ登りができるか確認する
    const bool GetIsClimbing() const { return m_canClimbing; }

    const Transform& GetTransform() const { return m_transform; }

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
    /// しゃがみ状態を解除できるかどうかを確認する(ステート)
    /// </summary>
    void CanUncrouch();

    /// <summary>
    /// よじ登りができるかを確認する
    /// </summary>
    /// <returns></returns>
    const bool CanClimbing();

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
    void GroundMove(const float speed);

    /// <summary>
    /// ウォールラン処理
    /// </summary>
    void WallRun();

    /// <summary>
    /// スライディング処理
    /// </summary>
    void Sliding();

    /// <summary>
    /// クライミング(よじ登り)処理
    /// </summary>
    void Climbing();

    /// <summary>
    /// クライミング(よじ登り)開始処理
    /// </summary>
    void StartClimbing();

    /// <summary>
    /// ジャンプ処理
    /// </summary>
    void JumpStart();

    /// <summary>
    /// 衝突判定の適用
    /// </summary>
    void ApplyCollision();

    /// <summary>
    /// 重力の適用
    /// </summary>
    void ApplyGravity();

    /// <summary>
    /// 移動量の更新
    /// </summary>
    void UpdateVelocity();

    /// <summary>
    /// カメラのParent設定処理
    /// </summary>
    void UpdateCameraParent();

    /// <summary>
    /// カメラ効果の適用
    /// </summary>
    void ApplyCameraEffect();

    /// <summary>
    /// モデルのアニメーションを更新する
    /// </summary>
    void UpdateModelAnimation();

    // デバッグUIの更新
#ifndef NDEBUG
    void UpdateDebugUI();
    void ToggleGodMode() { m_godMode = !m_godMode; }
    void MovementGodMode();
#endif // !NDEBUG


private:
    bool m_IsDead = false;
    bool m_IsFreeze = false;

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

    float m_crouchTimer = 0.0f;
    float m_crouchTime = 0.08f;

    //==================================================
    // モデル・カメラ・入力
    //==================================================
    std::unique_ptr<Object3d> m_pDrawModel;
    std::unique_ptr<Object3d> m_pModel;
    Camera* m_pCamera = nullptr;
    Input* m_pInput;
    PlayerCommand m_command;


    //==================================================
    // プレイヤー状態管理
    //==================================================
    std::unique_ptr<PlayerState> m_pCurrentState;
    const float& GetRunSpeed() const { return m_runSpeed; }
    const float& GetCrouchSpeed() const { return m_crounchSpeed; }
    const bool& IsMoveInput() const { return m_isMoveInput; }
    const bool& IsRunInput() const { return m_command.run; }
    const bool& IsCrouchInput() const { return m_command.crouch; }

    bool m_isFirstInput = false;
    WalkState m_state = WalkState::Idle;
    WalkState m_statePre = WalkState::Idle;

    friend class RunState;
    friend class JumpState;
    friend class CrouchState;

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

    bool m_isMoveInput = false;

    //================
    // ウォールラン関連
    //================
    AABB m_wallRunningObjectAABB = AABB::Zero;
    bool m_wallRunning = false;
    float m_wallRunGravity = -0.3f;
    Vector3 m_wallRunDirection = Vector3::Zero;
    Vector3 m_wallPenetration = Vector3::Zero;
    bool m_isStartWallRun = false;
    bool m_completeRotate = false;
    bool m_completeGetRotateInfo = false;
    float m_wallRunRotateAfter = 0.0f;
    float m_wallRunRotateAngle = SwapRadian(15.0f);
    float m_wallRunTimer = 0.0f;
    float m_wallRunTime = 0.14f;

    //================
    // しゃがみ関連
    //================
    float m_playerHeight = 0.0f;
    float m_cameraHeight = 1.5f;
    float m_crouchHeight = 0.0f;
    float m_crouchHeightOffset = -0.8f;

    //================
    // スライディング関連
    //================
    bool m_sliding = false;

    //================
    // よじ登り関連
    //================
    float m_canClimbingCheckSize = 2.5f; // よじ登り可能かのチェック範囲の増加量

    bool m_canClimbing = false;
    float m_climbingHeight = -4.0f; // よじ登りができるまでの高さ
    bool m_isClimbingMotion = false; 
    int m_climbingStep = 0; // よじ登りのステップ
    Vector3 m_climbingStartTranslate = Vector3::Zero;
    Vector3 m_climbingTopTranslate = Vector3::Zero;
    Vector3 m_climbingEndTranslate = Vector3::Zero;
    float m_climbingTimer = 0.0f;
    float m_climbingTime = 0.25f;

    // 移動速度
    const float m_runSpeed = 12.0f;
    const float m_crounchSpeed = 6.0f;
    float m_moveSpeed = 1.0f;
    float m_decelMoveSpeed = 1.0f;
    float m_moveSpeedPre = 1.0f;
    float m_playerSpeed = 0.0f; // 現在の速度
    float m_jumpHeight = 3.0f; // 最終的なジャンプ高さ

    //==================================================
    // カメラ関連
    //==================================================
    Transform m_cameraTransform = Transform::Default;
    Transform m_cameraVelocity = Transform::Default;

    // モデルの身長から目の位置までの割合
    float m_eyeHeight = 0.09f;

    // Fov補間用タイマー
    bool m_isFovChange = false;
    bool m_isRunFov = false;
    float m_fovBefore = 0.0f;
    float m_fovAfter = 0.0f;
    float m_fov = 0.0f;
    float m_fovPre = 0.0f;
    float m_fovChangeTimer = 0.0f;    // FOV補間用タイマー
    float m_fovChangeTime = 0.1f;     // FOV補間時間
    float m_fovDefault = 1.2f; // デフォルトFOV
    float m_fovRun = 1.4f;    // ダッシュ時FOV


    //==================================================
    // デバッグ関連（デバッグビルドのみ）
    //==================================================
#ifndef NDEBUG
    bool m_debugMode = false;
    bool m_godMode = false;
#endif // !NDEBUG

};

