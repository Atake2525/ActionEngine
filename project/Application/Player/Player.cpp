#include "Player.h"
#include "JsonLoader.h"
#include "Camera.h"
#include "Input.h"
#include "ImGuiManager.h"
#include "kMath.h"
#include "GameTime.h"
#include "CollisionManager.h"
#include <algorithm>
#include "EasingUtility.h"
#include <thread>
#include "SettingManager.h"
#include "SceneManager.h"
#include "ModelManager.h"
#include "RunState.h"
#include "JumpState.h"
#include "CrouchState.h"
#include "ClimbingState.h"
#include "WallRunState.h"
#include "WallJumpState.h"
#include "AirControlState.h"
#include "EngineContext.h"


using namespace std;


// 斜め移動の速度補間
constexpr float INV_SQRT2 = 0.70710678f; // 1 / sqrt(2)

Player::~Player()
{
    if (m_pContext)
    {
        m_pContext->world.collision.DeleteCollisionTarget("Player");
    }
}

void Player::Initialize(Camera* camera, const std::string& jsonName)
{
    AppContext& ctx = *m_pContext;
    // カメラのセット
    m_pCamera = camera;
    // カメラの初期設定
    m_pCamera->SetFovY(m_fovDefault);
    m_fov = m_fovDefault;
    m_fovBefore = m_fov;
    m_fovAfter = m_fovDefault;

    // JsonDataからステージ情報を取得してプレイヤーの初期位置とゴールの位置を設定する処理
    if (ctx.engine.assets.json.CheckJsonLoaded(jsonName))
    {
        // スタート地点の取得
        vector<JsonData> data = ctx.engine.assets.json.GetJsonData(jsonName, "startpoint");
        // スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
        if (!data.empty())
        {
            m_transform = data[0].transform;
            m_firstTransform = data[0].transform;
        }
    }

    Model* model = ctx.engine.assets.models.LoadModel("Resources/Model/obj/Player", "PlayerCollision.obj", false);
    // プレイヤーモデルの初期化
    m_pModel = ctx.game.object3dFactory.Create();
    m_pModel->SetModel(model);
    m_pModel->SetTransform(m_transform);
    //m_pModel->CreateCapsule();

    model = ctx.engine.assets.models.LoadModel("Resources/Model/gltf/char", "noHeadIdle.gltf", true);
    m_pDrawModel = ctx.game.object3dFactory.Create();
    m_pDrawModel->SetModel(model);
    // 初期モデル以外の移動アニメーションも起動時にまとめて読み込んでおく。
   /* m_pDrawModel->AddAnimationsThreaded("Resources/Model/gltf/char", {
        "sneak.gltf",
        "walk.gltf",
        "walk_back.gltf",
        "crouch.gltf",
        "dash.gltf",
        "fall.gltf",
        });*/
    m_pDrawModel->ToggleStartAnimation();

    m_playerAABB = m_pModel->GetAABB();
    m_playerAABB += m_transform.position;
    m_playerHeight = AABB::GetSize(m_playerAABB).y;
    ctx.world.collision.AddCollisionTarget(m_playerAABB, "Player");

    // カメラの高さをモデルの高さに合わせて調整 (ちょっとだけ低くする)
    m_cameraBaseTransform.position.y = m_playerAABB.max.y - m_transform.position.y - AABB::GetSize(m_playerAABB).y * m_eyeHeight;

    // コントロールモードの初期設定
    if (ctx.engine.platform.input.IsConnectedController())
    {
        m_controlMode = ControlMode::Gamepad;
    }
    else
    {
        m_controlMode = ControlMode::KeyboardMouse;
    }

    m_pInput = &ctx.engine.platform.input;

    // デバッグ用の初期設定
#ifndef NDEBUG
    m_pInput->ShowMouseCursor(true);
#else
    m_pInput->ShowMouseCursor(false);
#endif // !NDEBUG
    m_pCurrentState = std::make_unique<RunState>();
    m_pCurrentState->Enter(*this);

    Update();
}

void Player::Update()
{
    if (!m_IsFreeze)
    {
        m_transform = m_pModel->GetTransform();
        m_playerAABB = m_pModel->GetAABB();

        m_delta = m_pContext->engine.platform.time.GetDeltaTime();

        m_fov = m_pCamera->GetfovY();

#ifndef NDEBUG

        if (m_pInput->TriggerKey(DIK_F3))
        {
            m_debugMode = !m_debugMode;
        }
        if (m_debugMode)
        {
            // デバッグUIの更新
            UpdateDebugUI();
        }
#endif // !NDEBUG

#ifndef NDEBUG
        // デバッグビルドの処理

        if (m_godMode)
        {
            MovementGodMode();
        }
        else
        {
            // 入力の処理
            HandleInput();
            // 状態の更新
            UpdateState();
            // 回転処理
            Rotate();
            // 移動処理
            Move();
            ApplyCollision();

            // 重力の適用
            ApplyGravity();
        }

#else
        // リリースモードの処理

        // 入力の処理
        HandleInput();
        // 状態の更新
        UpdateState();
        // 回転処理
        Rotate();
        // 移動処理
        Move();
        ApplyCollision();

        // 重力の適用
        ApplyGravity();
#endif // !NDEBUG

        UpdateModelAnimation();

        m_transform.rotate += m_velocity.rotate;
        m_pModel->SetTransform(m_transform);

        if (!m_isFirstInput)
        {
            m_pModel->SetTransform(m_firstTransform);
        }

        m_pDrawModel->SetTransform(m_transform);
        Vector3 modelRotate = m_cameraBaseTransform.rotate;
        modelRotate.x = 0.0f;
        m_pDrawModel->SetRotate(modelRotate);
        ApplyCameraEffect();
        m_pModel->Update();
        m_pCamera->SetTransform(m_cameraBaseTransform + m_cameraEffectTransform);
        UpdateCameraParent();
    }
}

void Player::ChangeState(std::unique_ptr<PlayerBaseState> nextState)
{
    if (m_pCurrentState && nextState && nextState->GetStateId() == m_pCurrentState->GetStateId())
    {
        return;
    }

    if (m_pCurrentState)
    {
        m_pCurrentState->Exit(*this);
    }

    m_pCurrentState = std::move(nextState);

    if (m_pCurrentState)
    {
        m_pCurrentState->Enter(*this);
    }
}

void Player::UpdateModel() {
    m_pDrawModel->Update();
}

void Player::Draw()
{
    //m_pDrawModel->Draw();
}

void Player::UpdateState()
{
    // 前回の状態を保存
    m_statePre = m_state;



    // よじ登り中はClimbingに固定する
    if (m_isClimbingMotion)
    {
        m_canClimbing = false;
        m_onGround = true;
        m_state = WalkState::Idle;
        return;
    }

    // 入力が入ったら最初の入力フラグを立てる
    if (m_command.move.x != 0.0f || m_command.move.y != 0.0f)
    {
        m_isFirstInput = true;
    }

    if (m_onGround)
    {
        int statePoint = static_cast<int>(Sign((Sign(m_command.move.x) * m_command.move.x) + (Sign(m_command.move.y) * m_command.move.y)));
        m_state = static_cast<WalkState>(statePoint);
    }
    // よじ登りができるかの確認
    m_canClimbing = CanClimbing();

    // 空中判定(地面からの距離が一定以上離れているか)
    float groundDistance = m_pContext->world.collision.GetMaxGroundDistanceForAABB(m_playerAABB);
    if (groundDistance > 0.2f && m_onGround)
    {
        m_onGround = false;
        m_state = WalkState::Falling;
    }
    // 地面空の高さがほぼ0になっていれば地上判定を行う
    if (!m_onGround && m_velocity.position.y <= 0.0f && groundDistance <= 0.01f)
    {
        m_onGround = true;
    }

    if (m_onGround && m_isWallRunning)
    {
        ChangeState(std::make_unique<RunState>());
    }
    if (!m_onGround && !m_isWallRunning)
    {
        ChangeState(std::make_unique<AirControlState>());
    }

    if (m_command.crouch && !m_isClimbing && !m_isWallRunning)
    {
        ChangeState(std::make_unique<CrouchState>());
    }
    else if (CanUncrouch() && m_pCurrentState->GetStateId() == PlayerStateId::Crouch)
    {
        ChangeState(std::make_unique<RunState>());
    }
    UpdateParkourState();

}

void Player::UpdateParkourState()
{
    // 落下中かつオブジェクトに衝突している、移動方向が前に向いている時に壁走り
    if (CheckWallRunStart())
    {
        m_isWallRunning = true;
    }
    // 判定していなければウォールランを終了する
    if (CheckWallRunEnd())
    {
        m_isWallRunning = false;
        m_wallPenetration = Vector3::Zero;

    }

    // よじ登りの開始を確認する
    if (m_command.jump && m_canClimbing && !m_isClimbingMotion && !m_isClimbing)
    {
        m_isClimbing = true;
    }

    if (m_isWallRunning)
    {
        ChangeState(std::make_unique<WallRunState>());
    }

    // ジャンプ入力があったらジャンプ処理
    if (m_command.jump)
    {
        if (m_onGround)
        {
            ChangeState(std::make_unique<JumpState>());
        }
        if (m_isWallRunning)
        {
            ChangeState(std::make_unique<WallJumpState>());
        }
        if (m_isClimbing)
        {
            ChangeState(std::make_unique<ClimbingState>());
        }
    }
}

const bool Player::CheckWallRunStart()
{
    if (m_onGround)
    {
        return false;
    }
    if (m_isWallRunning || m_isClimbing)
    {
        return false;
    }
    // 衝突をしている必要があるので、貫通状態修正前の状態で衝突しているかを確認する
    AABB pAABB = m_playerAABB;
    pAABB += Vector3{ m_velocity.position.x, 0.0f, m_velocity.position.z };
    if (m_pContext->world.collision.IsCollisionObjectForAABB(pAABB) && m_velocity.position.y < 0.0f)
    {
        // 現状ウォールランは単一ベクトルなので貫通量と視点方向とある程度の移動量から移動方向を算出
        Vector3 cameraDir = {
            std::sin(m_cameraBaseTransform.rotate.y),
            0.0f,
            std::cos(m_cameraBaseTransform.rotate.y)
        };

        // オブジェクトが視点の正面にある場合はウォールランはしないようにしたいのでその場合はRunStateに戻す
        if ((m_wallPenetration.x != 0.0f && std::fabs(cameraDir.z) < 0.1f) || (m_wallPenetration.z != 0.0f && std::fabs(cameraDir.x) < 0.1f))
        {
            return false;
        }
        return true;
    }
    return false;
}

const bool Player::CheckWallRunEnd()
{
    if (!m_isWallRunning)
    {
        return false;
    }
    if (m_onGround)
    {
        return true;
    }
    // 衝突をしている必要があるので、貫通状態修正前の状態で衝突しているかを確認する
    AABB pAABB = m_playerAABB;
    pAABB += Vector3{ m_velocity.position.x, 0.0f, m_velocity.position.z };
    pAABB += m_wallPenetration;
    if (!m_pContext->world.collision.IsCollisionObjectForAABB(pAABB))
    {
        return true;
    }
    Vector3 cameraDir = {
        std::sin(m_cameraBaseTransform.rotate.y),
        0.0f,
        std::cos(m_cameraBaseTransform.rotate.y)
    };
    if ((m_wallPenetration.x != 0.0f && std::fabs(cameraDir.z) < 0.1f) || (m_wallPenetration.z != 0.0f && std::fabs(cameraDir.x) < 0.1f))
    {
        return false;
    }
    return false;
}

bool Player::CanUncrouch()
{
    AABB pAABB = m_playerAABB;
    pAABB.max.y = pAABB.min.y + m_playerHeight;
    // オブジェクトと上方向に貫通していたらcrouchを続けるようにする
    float penetration = m_pContext->world.collision.GetAllPenetrationForAABB(pAABB).y;
    if (penetration != 0.0f)
    {
        return false;
    }
    return true;
}

const bool Player::CanClimbing()
{
    AABB pAABB = m_playerAABB;
    pAABB += m_velocity.position;
    // 壁に当たっていなくても良い様にプレイヤーのAABBを大きくして判定する
    pAABB = AddSize(pAABB, m_canClimbingCheckSize);
    pAABB.min.y = m_playerAABB.min.y + m_velocity.position.y;
    pAABB.max.y = m_playerAABB.max.y + m_velocity.position.y;

    float groundObjectDist = m_pContext->world.collision.GetHeightToTopForAABB(pAABB);
    // プレイヤーの身長よりもある程度高くないと処理しないようにする
    float height = pAABB.max.y - pAABB.min.y;
    if (height / 3.0f > -groundObjectDist)
    {
        return false;
    }

    Vector3 dir = m_pContext->world.collision.GetCollisionObjectDirectionForAABB(pAABB);
    Vector3 cameraDir = {
        std::sin(m_cameraBaseTransform.rotate.y),
        0.0f,
        std::cos(m_cameraBaseTransform.rotate.y)
    };

    Vector3 fsbsCameraDir;

    fsbsCameraDir.x = fabs(cameraDir.x);
    fsbsCameraDir.y = fabs(cameraDir.y);
    fsbsCameraDir.z = fabs(cameraDir.z);

    bool IsWatchingWall = false;
    // プレイヤーも壁の方向を向いているかを調べる
    if (dir.x < 0.0f && fsbsCameraDir.x * Sign(cameraDir.x) < -fsbsCameraDir.z)
    {
        IsWatchingWall = true;
    }
    if (dir.x > 0.0f && cameraDir.x > fsbsCameraDir.z)
    {
        IsWatchingWall = true;
    }
    if (dir.z < 0.0f && fsbsCameraDir.z * Sign(cameraDir.z) < -fsbsCameraDir.x)
    {
        IsWatchingWall = true;
    }
    if (dir.z > 0.0f && cameraDir.z > fsbsCameraDir.x)
    {
        IsWatchingWall = true;
    }

    // 壁の高さを見てm_climbingHeight以内だったらよじ登りできる
    if (m_climbingHeight < groundObjectDist && 0.0f > groundObjectDist && IsWatchingWall && m_state == WalkState::Falling)
    {
        return true;
    }


    return false;
}

void Player::HandleInput()
{
    auto keyBind = m_pContext->game.sceneManager.GetSettingManager().GetKeyConfig();
    // 移動入力のリセット
    m_command.move = Vector2::Zero;

    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:
        // キー入力による移動
        m_command.move.y += m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveForward));
        m_command.move.y += -m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveBack));
        m_command.move.x += -m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveLeft));
        m_command.move.x += m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveRight));

        m_command.jump = m_pInput->TriggerKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Jump)) != 0;
        m_command.crouch = m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Crouch)) != 0;
        m_command.run = m_pInput->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Run)) != 0;

        m_command.eye = m_pInput->GetMouseVel3() * keyBind.sensitivity.mouse * 0.01f;
        break;
    case Player::ControlMode::Gamepad:
        // ジョイスティック入力による移動
        m_command.move = m_pInput->GetLeftJoyStickVelocity();
        m_command.move.y *= -1.0f;

        // gamepadはDPadやスティックの入力の可能性もあるので対応する
        m_command.jump = m_pInput->TriggerButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Jump)) != 0;
        m_command.crouch = m_pInput->PushButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Crouch)) != 0;
        m_command.run = m_pInput->PushButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Run)) != 0;

        m_command.eye = m_pInput->GetRightJoyStickVelocity() * keyBind.sensitivity.controller * 0.1f;
    }
    if (keyBind.sensitivity.invertX)
    {
        m_command.eye.x *= -1.0f;
    }
    if (keyBind.sensitivity.invertY)
    {
        m_command.eye.y *= -1.0f;
    }


}

void Player::Rotate() {
    // 視点の回転
    m_cameraBaseTransform.rotate.x += m_command.eye.y;
    m_cameraBaseTransform.rotate.y += m_command.eye.x;

    m_cameraBaseTransform.rotate.x = std::clamp(m_cameraBaseTransform.rotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

}

void Player::Move() {

    if (m_pCurrentState)
    {
        m_pCurrentState->Update(*this);
    }



    // 空中制御を適用
    UpdateVelocity();

    m_playerAABB.max.y = m_playerAABB.min.y + m_playerHeight + m_crouchHeight;
}

void Player::HorizontalMove(const float speed, const float accelerationTime, const float decelerationTime) {
    // Y軸回転から前後左右の回転を取得する
    Vector3 forward = {
        std::sin(m_cameraBaseTransform.rotate.y),
        0.0f,
        std::cos(m_cameraBaseTransform.rotate.y)
    };
    Vector3 right = {
        std::cos(m_cameraBaseTransform.rotate.y),
        0.0f,
        -std::sin(m_cameraBaseTransform.rotate.y)
    };
    // 移動方向を作る
    Vector3 inputDir = right * m_command.move.x + forward * m_command.move.y;
    // 長さが1を超える場合は正規化する
    inputDir = Normalize(inputDir);

    // 空中での目標速度を計算する
    Vector3 targetVelocity = inputDir * speed * m_delta;

    Vector3 velocity = m_horizontalVelocity;
    // 現在の速度から目標速度へ、一定量だけ近づける
    Vector3 diff = targetVelocity - velocity;
    float distance = Length(diff);
    float deceleration = speed / decelerationTime;
    float acceleration = speed / accelerationTime;
    float step = acceleration * m_delta * m_delta;

    if (distance <= step)
    {
        velocity = targetVelocity;
    }
    else
    {
        velocity += diff / distance * step;
    }
    m_horizontalVelocity = velocity;

    m_playerSpeed = Length(velocity);

}

void Player::WallRunStart() {
    // ウォールランの開始フレームの情報から移動方向を決める
    // ウォールラン用のオブジェクトとの貫通量を取得
    AABB pAABB = m_playerAABB;
    pAABB += Vector3{ m_velocity.position.x, 0.0f, m_velocity.position.z };
    m_wallPenetration = m_pContext->world.collision.GetPenetrationForAABB(pAABB);



    // 一定の速度以上で落下していた場合落下速度を行くり落とすようにする
    if (m_velocity.position.y < m_wallRunFallThreshold)
    {
        m_isDecelVelY = true;
        // 落下速度に応じて落下速度が0になるまでの時間を計算する 最大はm_maxDecelVelYTime
        m_wallRunFallTime = std::fabs(m_velocity.position.y + 0.2f) * 2.0f;
        m_wallRunFallTime = std::min(m_wallRunFallTime, m_maxwallRunFallTime);
        if (m_wallRunFallTime == 0.0f) // m_decelVelYTimeが0.0fだとエラーになるため 0.05fとする
        {
            m_wallRunFallTime = 0.05f;
        }
        m_velYBefore = m_velocity.position.y;
    }
    else
    {
        m_verticalVelocity = 0.0f;
    }
    // 現状ウォールランは単一ベクトルなので貫通量と視点方向とある程度の移動量から移動方向を算出
    Vector3 cameraDir = {
        std::sin(m_cameraBaseTransform.rotate.y),
        0.0f,
        std::cos(m_cameraBaseTransform.rotate.y)
    };
    m_wallRunDirection.x = Sign(std::fabs(m_wallPenetration.z)) * Sign(cameraDir.x);
    m_wallRunDirection.z = Sign(std::fabs(m_wallPenetration.x)) * Sign(cameraDir.z);
}

void Player::WallRun() {

    if (m_isDecelVelY)
    {
        m_wallRunFallTimer += m_pContext->engine.platform.time.GetDeltaTime() / m_wallRunFallTime;
        m_wallRunFallTimer = std::min(m_wallRunFallTimer, 1.0f);

        m_verticalVelocity = Lerp(m_velYBefore, 0.0f, m_wallRunFallTimer);

        if (m_wallRunFallTimer == 1.0f)
        {
            m_isDecelVelY = false;
            m_wallRunFallTimer = 0.0f;
        }
    }
}

void Player::StartClimbing()
{
    // よじ登りの最終地点などの計算
    float height = m_playerAABB.max.y - m_playerAABB.min.y;

    AABB pAABB = m_playerAABB + m_velocity.position;
    pAABB = AddSize(pAABB, m_canClimbingCheckSize);
    pAABB.min.y = m_playerAABB.min.y;
    pAABB.max.y = m_playerAABB.max.y;

    Vector3 cameraDir = m_pCamera->GetDirection();
    Vector3 fabsCameraDir = cameraDir;

    fabsCameraDir.x = fabs(fabsCameraDir.x);
    fabsCameraDir.y = fabs(fabsCameraDir.y);
    fabsCameraDir.z = fabs(fabsCameraDir.z);

    if (fabsCameraDir.x > fabsCameraDir.z)
    {
        cameraDir = { 1.0f * Sign(cameraDir.x), 0.0f, 0.0f };
    }
    if (fabsCameraDir.z > fabsCameraDir.x)
    {
        cameraDir = { 0.0f, 0.0f, 1.0f * Sign(cameraDir.z) };
    }

    AABB colObj = m_pContext->world.collision.GetObjectForCollisionDirection(pAABB, cameraDir);

    Vector3 rePairPosition = Vector3::Zero;
    if (cameraDir.x != 0.0f)
    {
        if (cameraDir.x > 0.0f)
        {
            rePairPosition.x = m_transform.position.x - colObj.min.x;
        }
        if (cameraDir.x < 0.0f)
        {
            rePairPosition.x = m_transform.position.x - colObj.max.x;
        }
    }
    if (cameraDir.z != 0.0f)
    {
        if (cameraDir.z > 0.0f)
        {
            rePairPosition.z = m_transform.position.z - colObj.min.z;
        }
        if (cameraDir.z < 0.0f)
        {
            rePairPosition.z = m_transform.position.z - colObj.max.z;
        }
    }

    float groundObjectDist = m_pContext->world.collision.GetHeightToTopForAABB(pAABB);
    rePairPosition.y = groundObjectDist;

    m_climbingStartPosition = m_transform.position;
    m_climbingEndPosition = m_transform.position - rePairPosition;
    m_climbingTopPosition = m_climbingStartPosition + ((m_climbingEndPosition - m_climbingStartPosition) * 0.35f);
    m_climbingTopPosition.y = max(m_climbingStartPosition.y, m_climbingEndPosition.y) + max(height * 0.15f, 0.35f);

    m_climbingTimer = 0.0f;
    m_climbingStep = 0;
    m_isClimbingMotion = true;
    m_state = WalkState::Idle;
    m_isWallRunning = false;
    m_wallPenetration = Vector3::Zero;
    m_velocity.position = Vector3::Zero;
}

void Player::Climbing()
{
    // よじ登りの動作処理
    // Stepが進むと完了時間を早める
    m_climbingTimer += m_delta / m_climbingTime * float(m_climbingStep + 1);
    float t = clamp(m_climbingTimer, 0.0f, 1.0f);
    Vector3 climbingPosition = Vector3::Zero;

    if (m_climbingStep == 0)
    {
        climbingPosition = EaseInOut(m_climbingStartPosition, m_climbingTopPosition, t);
    }
    else if (m_climbingStep == 1)
    {
        climbingPosition = Lerp(m_climbingTopPosition, m_climbingEndPosition, t);
    }

    Vector3 moveDelta = climbingPosition - m_transform.position;
    m_transform.position = climbingPosition;
    m_playerAABB += moveDelta;
    m_velocity.position = Vector3::Zero;
    m_playerSpeed = 0.0f;
    //m_onGround = true;

    if (t >= 1.0f)
    {
        if (m_climbingStep == 1)
        {
            m_isClimbingMotion = false;
            //m_onGround = false;
            m_isClimbing = false;
            ChangeState(std::make_unique<RunState>());
        }
        m_climbingTimer = 0.0f;
        m_climbingStep++;
    }

}

void Player::JumpStart()
{
    // ジャンプ開始時の初速を計算
    float gravityPerFrame = max(-m_gravity.y * m_delta, 0.0f);
    float jumpStartVelocity = sqrtf(2.0f * gravityPerFrame * m_jumpHeight);
    if (m_onGround) // 設置状態のジャンプ処理
    {
        m_verticalVelocity = jumpStartVelocity;
    }
}

void Player::WallJumpStart() {
    // ジャンプ開始時の初速を計算 
    float gravityPerFrame = max(-m_gravity.y * m_delta, 0.0f);
    float jumpStartVelocity = sqrtf(2.0f * gravityPerFrame * m_jumpHeight);
    m_verticalVelocity = jumpStartVelocity;

    Vector3 wallJumpDir = {
        -m_wallPenetration.x,
        0.0f,
        -m_wallPenetration.z
    };
    if (Length(wallJumpDir) > 0.0f)
    {
        wallJumpDir = Normalize(wallJumpDir);
        m_horizontalVelocity += wallJumpDir * 0.7f;
    }

    m_isWallRunning = false;
}

void Player::ApplyCollision()
{
    m_pContext->world.collision.UpdateCollisionTarget(m_playerAABB, "Player");
    m_pContext->world.collision.Update("Player");
    m_transform.position += m_velocity.position;
    Vector3 penetration = m_pContext->world.collision.GetPenetration();
    m_transform.position -= penetration;
    m_playerAABB -= penetration;
}

void Player::ApplyGravity()
{
    // 重力の適用処理の実装
    if (!m_onGround && !m_isWallRunning) // 空中にいてもウォールラン中ならば重力の処理は実行しない
    {
        float fallVelocity = m_verticalVelocity + m_gravity.y * m_delta;
        float groundDist = m_pContext->world.collision.GetMaxGroundDistanceForAABB(m_playerAABB);
        // 落下速度の上限
        m_verticalVelocity += m_gravity.y * m_delta;
        m_verticalVelocity = max(fallVelocity, -groundDist);
    }
}

void Player::UpdateVelocity()
{
    // 移動方向の計算
    m_velocity.position = m_horizontalVelocity;
    m_velocity.position.y = m_verticalVelocity;
    m_playerAABB += m_velocity.position;
}

void Player::UpdateCameraParent() {
    // カメラのParent設定処理の実装
    m_pCamera->SetParent(m_pModel->GetWorldMatrix());
}

void Player::WallRunCameraTiltStart() {
    // ウォールランの進行方向にカメラを合わせる(補助)を行うために角度を計算する
    // 高さを考慮する必要はないので二次元空間でXとZを使って計算する
    // 二次元空間で0は右なので0を正面にするためatan2fの計算はそのままx,zにする
    m_wallRunningTargetCameraRotateY = atan2f(m_wallRunDirection.x, m_wallRunDirection.z);
    float cameraRotY = std::fmod(m_cameraBaseTransform.rotate.y, 2 * std::numbers::pi_v<float>);
    m_wallRunningBeforeCameraRotateY = cameraRotY;
}

void Player::UpdateWallRunCameraTilt() {
    // ウォールラン中のカメラの傾き処理の実装
    if (m_isWallRunning)
    {
        if (!m_completeGetRotateInfo)
        {
            m_completeGetRotateInfo = true;
            Vector3 signWallRunDirection = { 1.0f, 1.0f, 1.0f };
            if (m_wallRunDirection.x < 0.0f)
            {
                signWallRunDirection.x = -1.0f;
            }
            if (m_wallRunDirection.z < 0.0f)
            {
                signWallRunDirection.z = -1.0f;
            }
            float wallRunPenetration = -1.0f;
            if (m_wallPenetration.z > 0.0f || m_wallPenetration.x < 0.0f)
            {
                wallRunPenetration = 1.0f;
            }
            // 回転後角度を代入 回転後の角度は移動方向、壁がプレイヤーから左右どちらにあるかによって変わるのでそれも考慮する
            // 移動方向に応じての回転を入れいる
             // 現状ウォールランは単一ベクトルなので貫通量と視点方向とある程度の移動量から移動方向を算出

            if (m_wallRunDirection.x != 0.0f)
            {
                m_wallRunRotateZ = m_wallRunRotateAngle * (signWallRunDirection.x * wallRunPenetration * -1.0f);
            }
            if (m_wallRunDirection.z != 0.0f)
            {
                m_wallRunRotateZ = m_wallRunRotateAngle * (signWallRunDirection.z * wallRunPenetration * -1.0f);
            }
        }
        m_wallRunRotateZTimer += m_delta / m_wallRunRotateZTime;
        m_wallRunRotateYTimer += m_delta / m_wallRunRotateYTime;

        float cameraRotY = std::fmod(m_cameraBaseTransform.rotate.y, 2 * std::numbers::pi_v<float>);
        
        if (Length(m_command.eye) != 0.0f)
        {
            m_wallRunningBeforeCameraRotateY = cameraRotY;
            m_wallRunRotateYTimer = 0.0f;
        }
        else // 視点移動がないときにのみ補助が働くようにする
        {
            constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;
            float angleDiff = std::remainder(m_wallRunningTargetCameraRotateY - cameraRotY, twoPi);

            if (std::fabs(angleDiff) <= m_wallRunningRotateYSpeed)
            {
                m_cameraBaseTransform.rotate.y = m_wallRunningTargetCameraRotateY;
            }
            else
            {
                m_cameraBaseTransform.rotate.y += std::copysign(m_wallRunningRotateYSpeed, angleDiff);
            }
        }
    }
    else
    {
        m_completeGetRotateInfo = false;
        m_wallRunRotateZTimer -= m_delta / m_wallRunRotateZTime;
        m_wallRunRotateYTimer = 0.0f;
    }
    m_wallRunRotateZTimer = clamp(m_wallRunRotateZTimer, 0.0f, 1.0f);
    m_wallRunRotateYTimer = clamp(m_wallRunRotateYTimer, 0.0f, 1.0f);





    m_cameraEffectTransform.rotate.z = Lerp(0.0f, m_wallRunRotateZ, m_wallRunRotateZTimer);
}

void Player::UpdateCameraFov() {
    // Fov変更処理の実装
    m_fov = m_pCamera->GetfovY();
    m_fovPre = m_fov;
    // 移動速度がダッシュ速度ならFovを広げる
    if (m_playerSpeed >= m_runSpeed && !m_isRunFov)
    {
        m_fovChangeTimer = 0.0f;
        m_fovBefore = m_fov;
        m_fovAfter = m_fovRun;
        m_isRunFov = true;
    }
    if (m_isRunFov && m_playerSpeed == 0.0f)
    {
        m_fovChangeTimer = 0.0f;
        m_fovBefore = m_fov;
        m_fovAfter = m_fovDefault;
        m_isRunFov = false;
    }
    m_fovChangeTimer += m_pContext->engine.platform.time.GetDeltaTime() / m_fovChangeTime;
    m_fovChangeTimer = clamp(m_fovChangeTimer, 0.0f, 1.0f);
    m_fov = Lerp(m_fovBefore, m_fovAfter, m_fovChangeTimer);
    // 計算結果をカメラにセット
    m_pCamera->SetFovY(m_fov);
}

void Player::UpdateCrouchCamera() {
    if (m_command.crouch)
    {
        m_crouchTimer += m_delta / m_crouchTime;
    }
    else
    {
        m_crouchTimer -= m_delta / m_crouchTime;
    }

    m_crouchTimer = clamp(m_crouchTimer, 0.0f, 1.0f);
    m_crouchHeight = Lerp(0.0f, m_crouchHeightOffset, m_crouchTimer);

    // Crouch時のカメラの高さ変更処理の実装
    m_crouchCameraOffsetY = m_crouchHeight + m_cameraHeight;
    m_cameraBaseTransform.position.y = m_crouchCameraOffsetY;
}

void Player::UpdateHeadBob() {
    // テスト実装

    // m_horizontalVelocity は1フレームの移動量なので秒速に戻す
    float horizontalSpeed =
        Length(Vector3{
            m_horizontalVelocity.x,
            0.0f,
            m_horizontalVelocity.z
            }) / max(m_delta, 0.0001f);

    float speedRate = clamp(horizontalSpeed / m_runSpeed, 0.0f, 1.0f);

    // 停止中でも上下し、移動時ほど速くする
    const float idleFrequency = 1.5f;
    const float moveFrequency = 10.0f;
    float frequency = Lerp(idleFrequency, moveFrequency, speedRate);

    // 停止中は小さく、移動中は少し大きくする
    const float idleAmplitude = 0.015f;
    const float moveAmplitude = 0.08f;
    float amplitude = Lerp(idleAmplitude, moveAmplitude, speedRate);

    m_headBobTimer += m_delta * frequency;

    float bobY = std::sin(m_headBobTimer) * amplitude;

    // カメラへ反映
    m_cameraEffectTransform.position.y = bobY;
}

void Player::ApplyCameraEffect()
{
    UpdateCrouchCamera();
    UpdateCameraFov();
    UpdateWallRunCameraTilt();
    UpdateHeadBob();

}

void Player::UpdateModelAnimation() {
    std::string animName = "DefaultAnimation";

    if (m_state == WalkState::Falling)
    {
        animName = "fall";
    }
    else if (m_state == WalkState::Idle)
    {
        animName = "DefaultAnimation";
    }

    if (m_pDrawModel->GetCurrentAnimationKey() != animName)
    {
        m_pDrawModel->ChangePlayAnimation(animName);
    }
}

#ifndef NDEBUG
void Player::UpdateDebugUI() {

    if (!ImGui::Begin("Player Debug")) {
        ImGui::End();
        return;
    }
    // --- God Mode ---
    if (ImGui::Checkbox("God Mode", &m_godMode) && !m_godMode)
    {
        m_cameraBaseTransform.position = Vector3::Zero;
    }

    // --- Control Mode ---
    ImGui::Text("Control Mode: %s",
        m_controlMode == ControlMode::KeyboardMouse ? "Keyboard & Mouse" : "Gamepad");
    // Control Mode 切り替え
    if (ImGui::Button("Toggle Control Mode"))
    {
        if (m_controlMode == ControlMode::KeyboardMouse)
        {
            m_controlMode = ControlMode::Gamepad;
        }
        else
        {
            m_controlMode = ControlMode::KeyboardMouse;
        }
    }

    // --- Player State ---
    ImGui::Text("State: %d", static_cast<int>(m_state));
    ImGui::Text("Prev State: %d", static_cast<int>(m_statePre));

    static const char* stateItems[] = { "Idle", "Move", "Falling" };
    int stateIndex = static_cast<int>(m_state);


    ImGui::Text("PlayerMoveState: %s", stateItems[stateIndex]);

    static const char* statePatterns[] = { "Run", "Crouch", "Jump", "WallRun", "WallJump", "Climbing", "AirControl" };
    stateIndex = static_cast<int>(m_pCurrentState->GetStateId());
    ImGui::Text("PlayerState: %s", statePatterns[stateIndex]);

    ImGui::Checkbox("OnGround", &m_onGround);

    ImGui::Checkbox("WallRun", &m_isWallRunning);

    ImGui::Checkbox("CanClimbing", &m_canClimbing);

    ImGui::DragFloat("CrouchHeihgt", &m_crouchHeight, 0.0f);

    ImGui::DragFloat3("WallRunDirection", &m_wallRunDirection.x, 0.1f);

    ImGui::Separator();

    // --- Movement ---
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::Text("Move Input: (%f, %f)", m_command.move.x, m_command.move.y);

        ImGui::Text("Gravity: (%.2f, %.2f, %.2f)",
            m_gravity.x, m_gravity.y, m_gravity.z);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 0.1f, 0.0f, 100.0f);

        ImGui::DragFloat("Jump Height", &m_jumpHeight, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat3("Gravity", &m_gravity.x, 0.1f);

        float groundDist = m_pContext->world.collision.GetMaxGroundDistanceForAABB(m_playerAABB);
        ImGui::Text("Ground Distance: %.2f", groundDist);

        AABB pAABB = m_playerAABB + m_velocity.position;
        groundDist = m_pContext->world.collision.GetGroundDistanceForAABB(pAABB);
        ImGui::Text("Ground Object Distance: %.2f", groundDist);
        // 貫通量
        Vector3 penetration = m_pContext->world.collision.GetPenetration();
        ImGui::Text("Penetration: (%.2f, %.2f, %.2f)",
            penetration.x, penetration.y, penetration.z);

        AABB aabb = m_playerAABB + Vector3{ m_velocity.position.x, 0.0f, m_velocity.position.z };
        penetration = m_pContext->world.collision.GetPenetrationForAABB(aabb);
        ImGui::Text("WallRun Penetration: (%.2f, %.2f, %.2f)", penetration.x, penetration.y, penetration.z);

        // Transform
        if (ImGui::TreeNode("Transform")) {
            ImGui::DragFloat3("##Position", &m_transform.position.x, 0.1f);
            ImGui::DragFloat3("##Rotate", &m_transform.rotate.x, 0.1f);
            ImGui::Text("Scale:(%.2f, %.2f, %.2f)",
                m_transform.scale.x, m_transform.scale.y, m_transform.scale.z);
            ImGui::TreePop();
        }

        // Velocity
        if (ImGui::TreeNode("Velocity")) {
            ImGui::DragFloat3("##Velocity Position", &m_velocity.position.x, 0.1f);
            ImGui::DragFloat3("##Vel Rotate", &m_velocity.rotate.x, 0.1f);
            float moveSpeed = Length({ m_velocity.position.x, 0.0f, m_velocity.position.z });
            ImGui::Text("Speed : %f", moveSpeed);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Speed")) {
            // プレイヤーの速度
            float speed = m_playerSpeed / m_delta;
            ImGui::Text("Speed : %f", speed);
            // 歩行速度
            ImGui::Text("Run Speed : %f", &m_runSpeed);
            ImGui::Text("Crounch Speed : %f", &m_crounchSpeed);
            // 減速時間
            ImGui::DragFloat("Decel Time", &m_decelTime, 0.01f);
            ImGui::TreePop();
        }

        if (ImGui::CollapsingHeader("GamePad")) {
            // 左スティック入力
            Vector2 leftStick = m_pInput->GetLeftJoyStickPos2(0.0f) / 1000.0f;
            ImGui::Text("Left Stick: (%.2f, %.2f)",
                leftStick.x, leftStick.y);
            // 右スティック入力
            Vector3 rightStick = m_pInput->GetRightJoyStickPos3(0.0f);
            ImGui::Text("Right Stick: (%.2f, %.2f, %.2f)",
                rightStick.x, rightStick.y, rightStick.z);

        }
    }

    ImGui::Separator();

    // --- Camera ---
    if (ImGui::CollapsingHeader("Camera")) {

        if (ImGui::TreeNode("Base"))
        {
            ImGui::Text("Transform");
            ImGui::Separator();
            ImGui::Text("Pos:  (%.2f, %.2f, %.2f)",
                m_cameraBaseTransform.position.x, m_cameraBaseTransform.position.y, m_cameraBaseTransform.position.z);
            ImGui::Text("Rot:  (%.2f, %.2f, %.2f)",
                m_cameraBaseTransform.rotate.x, m_cameraBaseTransform.rotate.y, m_cameraBaseTransform.rotate.z);

            ImGui::Text("Velocity");
            ImGui::Text("Vel Pos: (%.2f, %.2f, %.2f)",
                m_cameraVelocity.position.x, m_cameraVelocity.position.y, m_cameraVelocity.position.z);
            ImGui::Text("Vel Rot: (%.2f, %.2f, %.2f)",
                m_cameraVelocity.rotate.x, m_cameraVelocity.rotate.y, m_cameraVelocity.rotate.z);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Effect"))
        {
            ImGui::Text("Transform");
            ImGui::Separator();
            ImGui::Text("Pos:  (%.2f, %.2f, %.2f)",
                m_cameraEffectTransform.position.x, m_cameraEffectTransform.position.y, m_cameraEffectTransform.position.z);
            ImGui::Text("Rot:  (%.2f, %.2f, %.2f)",
                m_cameraEffectTransform.rotate.x, m_cameraEffectTransform.rotate.y, m_cameraEffectTransform.rotate.z);
            ImGui::TreePop();
        }

        ImGui::SliderFloat("FOV", &m_fov, 30.0f, 120.0f);
    }

    ImGui::End();
}

void Player::MovementGodMode()
{
    // 視点移動
    m_cameraBaseTransform.rotate.x += m_pInput->GetMouseVel2().y * 0.001f;
    m_cameraBaseTransform.rotate.y += m_pInput->GetMouseVel2().x * 0.001f;

    // 移動処理
    float delta = m_pContext->engine.platform.time.GetDeltaTime();
    Vector3 moveDir = Vector3::Zero;
    // キーボード移動
    moveDir.z += -m_pInput->PushKeyInt(DIK_S);
    moveDir.z += m_pInput->PushKeyInt(DIK_W);
    moveDir.x += -m_pInput->PushKeyInt(DIK_A);
    moveDir.x += m_pInput->PushKeyInt(DIK_D);
    m_cameraBaseTransform.position.y += m_pInput->PushKeyInt(DIK_SPACE);
    m_cameraBaseTransform.position.y += -m_pInput->PushKeyInt(DIK_LCONTROL);

    // 視点方向に移動
    Transform dir = Transform::Default;
    dir.rotate = m_cameraBaseTransform.rotate;
    Vector3 moveVelocity = TransformNormal(moveDir, MakeAffineMatrix(dir));
    m_cameraVelocity.position = moveVelocity * delta * 20.0f;

    m_cameraBaseTransform.position += m_cameraVelocity.position;
}

#endif // !NDEBUG
