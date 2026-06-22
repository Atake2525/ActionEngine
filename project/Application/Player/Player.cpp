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


using namespace std;


// 斜め移動の速度補間
constexpr float INV_SQRT2 = 0.70710678f; // 1 / sqrt(2)

Player::~Player()
{
    CollisionManager::GetInstance()->DeleteCollisionTarget("Player");
}

void Player::Initialize(Camera* camera, const std::string& jsonName)
{
    // カメラのセット
    m_pCamera = camera;
    // カメラの初期設定
    m_pCamera->SetFovY(m_fovDefault);
    m_fov = m_fovDefault;

    // JsonDataからステージ情報を取得してプレイヤーの初期位置とゴールの位置を設定する処理
    if (JsonLoader::GetInstance()->CheckJsonLoaded(jsonName))
    {
        // スタート地点の取得
        vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(jsonName, "startpoint");
        // スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
        if (!data.empty())
        {
            m_transform = data[0].transform;
            m_firstTransform = data[0].transform;
        }
    }

    Model* model = ModelManager::GetInstance()->LoadModel("Resources/Model/obj/Player", "PlayerCollision.obj", false);
    // プレイヤーモデルの初期化
    m_pModel = make_unique<Object3d>();
    m_pModel->Initialize();
    m_pModel->SetModel(model);
    m_pModel->SetTransform(m_transform);
    //m_pModel->CreateCapsule();

    model = ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/char", "noHeadIdle.gltf", true);
    m_pDrawModel = make_unique<Object3d>();
    m_pDrawModel->Initialize();
    m_pDrawModel->SetModel(model);
    // 初期モデル以外の移動アニメーションも起動時にまとめて読み込んでおく。
    m_pDrawModel->AddAnimationsThreaded("Resources/Model/gltf/char", {
        "sneak.gltf",
        "walk.gltf",
        "walk_back.gltf",
        "crouch.gltf",
        "dash.gltf",
        "fall.gltf",
        });
    m_pDrawModel->ToggleStartAnimation();

    m_playerAABB = m_pModel->GetAABB();
    m_playerAABB += m_transform.translate;
    m_playerHeight = AABB::GetSize(m_playerAABB).y;
    CollisionManager::GetInstance()->AddCollisionTarget(m_playerAABB, "Player");

    // カメラの高さをモデルの高さに合わせて調整 (ちょっとだけ低くする)
    m_cameraTransform.translate.y = m_playerAABB.max.y - m_transform.translate.y - AABB::GetSize(m_playerAABB).y * m_eyeHeight;
    //m_cameraHeight = m_cameraTransform.translate.y;

    // コントロールモードの初期設定
    if (Input::GetInstance()->IsConnectedController())
    {
        m_controlMode = ControlMode::Gamepad;
    }
    else
    {
        m_controlMode = ControlMode::KeyboardMouse;
    }

    m_pInput = Input::GetInstance();

    // デバッグ用の初期設定
#ifndef NDEBUG
    Input::GetInstance()->ShowMouseCursor(true);
#else
    Input::GetInstance()->ShowMouseCursor(false);
#endif // !NDEBUG
    ChangeState(std::make_unique<RunState>());

    Update();
}

void Player::Update()
{
    m_transform = m_pModel->GetTransform();
    m_playerAABB = m_pModel->GetAABB();

    m_delta = GameTime::GetInstance()->GetDeltaTime();

    m_fov = m_pCamera->GetfovY();

#ifndef NDEBUG

    if (Input::GetInstance()->TriggerKey(DIK_F3))
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
    Vector3 modelRotate = m_cameraTransform.rotate;
    modelRotate.x = 0.0f;
    m_pDrawModel->SetRotate(modelRotate);
    ApplyCameraEffect();
    m_pModel->Update();
    m_pCamera->SetTransform(m_cameraTransform);
    UpdateCameraParent();
}

void Player::ChangeState(std::unique_ptr<PlayerState> nextState)
{
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
    m_walkStatePre = m_walkState;

    // よじ登り中はClimbingに固定する
    if (m_isClimbingMotion)
    {
        m_walkState = PlayerWalkState::Climbing;
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

    m_walkState = PlayerWalkState::Walk;

    // 空中判定(地面からの距離が一定以上離れているか)
    float groundDistance = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
    if (groundDistance > 0.2f && m_onGround)
    {
        m_onGround = false;
        m_state = WalkState::Falling;
    }
    // 地面空の高さがほぼ0になっていれば地上判定を行う
    if (!m_onGround && m_velocity.translate.y <= 0.0f && groundDistance <= 0.01f)
    {
        m_onGround = true;
        m_wallRunningObjectAABB = AABB::Zero;
    }

    // Run入力があり、かつ前回の状態がWalkであればRunにする
    if (m_command.run && (m_velocity.translate.z != 0.0f || m_velocity.translate.x != 0.0f))
    {
        //m_walkState = PlayerWalkState::Run;
        ChangeState(std::make_unique<RunState>());
    }
    // しゃがみ入力
    if (m_command.crouch)
    {
        //m_walkState = PlayerWalkState::Crouch;
        ChangeState(std::make_unique<CrouchState>());
    }

    // crouchを解除しようとしたときに可能かを確認する
    if (m_walkStatePre == PlayerWalkState::Crouch && m_walkState != PlayerWalkState::Crouch)
    {
        CanUncrouch();
    }

    UpdateParkourState();

    
}

void Player::UpdateParkourState()
{

    if (!m_onGround)
    {
        AABB pAABB = m_playerAABB;
        pAABB += Vector3{ m_velocity.translate.x, 0.0f, m_velocity.translate.z };
        // 落下中かつ壁走り用のオブジェクトに衝突している時に壁走り
        if (!m_wallRunning && CollisionManager::GetInstance()->IsCollisionObjectForAABB(pAABB, true, m_wallRunningObjectAABB) && m_velocity.translate.y < 0.0f)
        {
            if (CollisionManager::GetInstance()->GetCollisionObjectAABBsForAABB(pAABB, true).size() != 0)
            {
                m_wallRunningObjectAABB = CollisionManager::GetInstance()->GetCollisionObjectAABBsForAABB(pAABB, true)[0];
            }
            m_wallRunning = true;
        }
        if (m_wallRunning)
        {
            m_walkState = PlayerWalkState::WallRun;
        }
        // ウォールランの終了を確認する
        pAABB += m_wallPenetration;
        // 現在の位置(AABB)からウォールラン中の壁の方向に移動させ衝突しているかを判定する
        // 判定していなければウォールランを終了する
        if (m_wallRunning && m_isStartWallRun && (!CollisionManager::GetInstance()->IsCollisionObjectForAABB(pAABB, true) || m_command.jump == 1.0f))
        {
            m_wallRunning = false;
            m_isStartWallRun = false;
            m_wallPenetration = Vector3::Zero;
        }
    }

    // よじ登りの開始を確認する
    if (m_command.jump && m_canClimbing && !m_isClimbingMotion)
    {
        StartClimbing();
        m_walkState = PlayerWalkState::Climbing;
    }
}

void Player::CanUncrouch()
{
    AABB pAABB = m_playerAABB;
    pAABB.max.y = pAABB.min.y + m_playerHeight;
    // オブジェクトと上方向に貫通していたらcrouchを続けるようにする
    float penetration = CollisionManager::GetInstance()->GetAllPenetrationForAABB(pAABB).y;
    if (penetration != 0.0f)
    {
        m_walkState = PlayerWalkState::Crouch;
    }
}

const bool Player::CanClimbing()
{
    AABB pAABB = m_playerAABB;
    pAABB += m_velocity.translate;
    // 壁に当たっていなくても良い様にプレイヤーのAABBを大きくして判定する
    pAABB = AddSize(pAABB, m_canClimbingCheckSize);
    pAABB.min.y = m_playerAABB.min.y + m_velocity.translate.y;
    pAABB.max.y = m_playerAABB.max.y + m_velocity.translate.y;

    float groundObjectDist = CollisionManager::GetInstance()->GetHeightToTopForAABB(pAABB);
    // プレイヤーの身長よりもある程度高くないと処理しないようにする
    float height = pAABB.max.y - pAABB.min.y;
    if (height / 3.0f > -groundObjectDist)
    {
        return false;
    }

    Vector3 dir = CollisionManager::GetInstance()->GetCollisionObjectDirectionForAABB(pAABB);
    Vector3 cameraDir = m_pCamera->GetDirection();

    /*dir.x = fabs(dir.x);
    dir.y = fabs(dir.y);
    dir.z = fabs(dir.z);*/

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
    auto keyBind = SceneManager::GetInstance()->GetSettingManager().GetKeyConfig();
    // 移動入力のリセット
    m_command.move = Vector2::Zero;

    if (!m_IsFreeze)
    {
        switch (m_controlMode)
        {
        case Player::ControlMode::KeyboardMouse:
            // キー入力による移動
            m_command.move.y += Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveForward));
            m_command.move.y += -Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveBack));
            m_command.move.x += -Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveLeft));
            m_command.move.x += Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::MoveRight));

            m_command.jump = Input::GetInstance()->TriggerKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Jump)) != 0;
            m_command.crouch = Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Crouch)) != 0;
            m_command.run = Input::GetInstance()->PushKeyInt(keyBind.keyboardConfig.GetAction(Setting::Action::Run)) != 0;

            m_command.eye = Input::GetInstance()->GetMouseVel3() * keyBind.sensitivity.mouse * 0.01f;
            break;
        case Player::ControlMode::Gamepad:
            // ジョイスティック入力による移動
            m_command.move = m_pInput->GetLeftJoyStickVelocity();
            m_command.move.y *= -1.0f;

            // gamepadはDPadやスティックの入力の可能性もあるので対応する


            m_command.jump = Input::GetInstance()->TriggerButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Jump)) != 0;
            m_command.crouch = Input::GetInstance()->PushButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Crouch)) != 0;
            m_command.run = Input::GetInstance()->PushButton(keyBind.controllerConfig.GetControllerAction(Setting::Action::Run)) != 0;

            m_command.eye = Input::GetInstance()->GetRightJoyStickVelocity() * keyBind.sensitivity.controller * 0.1f;
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


}

void Player::Rotate() {
    m_cameraTransform.rotate = m_pCamera->GetTransform().rotate;
    // 視点の回転
    m_cameraTransform.rotate.x += m_command.eye.y;
    m_cameraTransform.rotate.y += m_command.eye.x;

    m_cameraTransform.rotate.x = std::clamp(m_cameraTransform.rotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

}

void Player::Move() {
    // よじ登り中は他の処理をしないように最初に更新
    if (m_walkState == PlayerWalkState::Climbing || m_isClimbingMotion)
    {
        Climbing();
        m_playerAABB.max.y = m_playerAABB.min.y + m_playerHeight + m_crouchHeight;
        return;
    }

    if (!m_wallRunning)
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->Update(*this);
        }
    }
    else
    {
        WallRun();
    }

    // ジャンプ入力があったらジャンプ処理
    if (m_command.jump)
    {
        ChangeState(std::make_unique<JumpState>());
        m_pCurrentState->Enter(*this);

    }

    if (m_walkState == PlayerWalkState::Climbing)
    {
        Climbing();
    }

    // 空中制御を適用
    UpdateVelocity();

    m_playerAABB.max.y = m_playerAABB.min.y + m_playerHeight + m_crouchHeight;
}

void Player::GroundMove(const float speed)
{
    m_crouchHeight = 0.0f;
    // m_command.move(入力)に基づいてプレイヤーの移動方向を決定
    if (m_command.move.x != 0.0f || m_command.move.y != 0.0f)
    {
        // 入力中は移動量を上げる
        m_moveAmount.x += Sign(m_command.move.x) * (m_delta / m_maxSpeedTime * m_runSpeed);
        m_moveAmount.y += Sign(m_command.move.y) * (m_delta / m_maxSpeedTime * m_runSpeed);
    }

    m_moveSpeedPre = m_moveSpeed;

    if (speed == m_crounchSpeed)
    {
        m_crouchTimer += m_delta / m_crouchTime;
    }


    if (m_crouchTimer != 0.0f && m_walkState != PlayerWalkState::Crouch)
    {
        m_crouchTimer -= m_delta / m_crouchTime;
    }

    m_crouchTimer = clamp(m_crouchTimer, 0.0f, 1.0f);
    m_crouchHeight = Lerp(0.0f, m_crouchHeightOffset, m_crouchTimer);

    // 移動速度の補間(速度が上がる時には即座に反映、速度が下がる時には時間をかけて反映)
    if (speed < m_moveSpeedPre && !m_isSpeedDecel)
    {
        m_speedBefore = m_moveSpeedPre;
        m_speedAfter = speed;
        m_isSpeedDecel = true;
    }
    if (m_moveSpeed < speed)
    {
        m_moveSpeed = speed;
        m_isSpeedDecel = false;
        m_moveSpeedTimer = 0.0f;
    }

    // 移動速度減速中のタイマー処理
    if (m_isSpeedDecel)
    {
        m_moveSpeedTimer += m_delta / m_speedDecelTime;
        float t = clamp(m_moveSpeedTimer, 0.0f, 1.0f);
        m_moveSpeed = Lerp(m_speedBefore, m_speedAfter, t);
    }
    // 減速が完了したらフラグとタイマーをリセット
    if (m_moveSpeedTimer >= 1.0f)
    {
        m_isSpeedDecel = false;
        m_moveSpeedTimer = 0.0f;
    }

    Vector2 fabsMoveInput = { fabs(m_command.move.x), fabs(m_command.move.y) };

    if (m_command.move.x == 0.0f)
    {
        fabsMoveInput.x = 1.0f;
    }
    if (m_command.move.y == 0.0f)
    {
        fabsMoveInput.y = 1.0f;
    }

    // 移動量のクランプ
    m_moveAmount.x = clamp(m_moveAmount.x, -m_moveSpeed * fabsMoveInput.x, m_moveSpeed * fabsMoveInput.x);
    m_moveAmount.y = clamp(m_moveAmount.y, -m_moveSpeed * fabsMoveInput.y, m_moveSpeed * fabsMoveInput.y);

    m_decelMoveSpeed = m_runSpeed;

    // 入力が無かったら一定速度で減速して0.0fにする
    // 減速は加速よりも早くする
    if (m_command.move.x == 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (m_command.move.y == 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
            m_moveAmount.y = 0.0f;
        }
    }
    // 反対入力があったら減速速度を上げる
    if (Sign(m_command.move.x) != Sign(m_moveAmount.x) && m_command.move.x != 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (m_delta / (m_decelTime / 2.0f) * m_runSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (m_delta / (m_decelTime / 2.0f) * m_runSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (Sign(m_command.move.y) != Sign(m_moveAmount.y) && m_command.move.y != 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (m_delta / (m_decelTime / 2.0f) * m_runSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (m_delta / (m_decelTime / 2.0f) * m_runSpeed))) {
            m_moveAmount.y = 0.0f;
        }
    }

    // 斜め移動補正（正規化）
    float len = Length(m_moveAmount);
    if (len > 1.0f) {
        m_moveAmount /= len;
    }

    // MoveDirectionをm_cameraTransform.rotateの向きに等速で合わせる
    if (m_moveDirection.y != m_cameraTransform.rotate.y) {
        // Y軸回転の差分を計算
        float diff = m_cameraTransform.rotate.y - m_moveDirection.y;
        // 地上と空中で回転速度を変える
        float turnFactor = m_turnControlFactor;
        if (!m_onGround)
        {
            turnFactor = m_airControlFactor;
        }
        // 角度の差分を-180度から180度の範囲に収める
        float adjust = Sign(diff) * m_moveSpeed * (m_delta / turnFactor);
        // 差分が調整量より小さい場合は直接合わせる
        if (abs(diff) < abs(adjust)) { // 調整量より差分が小さい場合
            m_moveDirection.y = m_cameraTransform.rotate.y;
        }
        else { // 調整量分だけ移動方向を回転させる
            m_moveDirection.y += adjust;
        }
    }
    m_moveDirection.y = m_cameraTransform.rotate.y;


    // 現在の速度を計算
    m_playerSpeed = max(fabs(m_moveAmount.x), fabs(m_moveAmount.y));
}

void Player::WallRun()
{
    if (!m_isStartWallRun)
    {
        // ウォールランの開始フレームの情報から移動方向を決める
        // ウォールラン用のオブジェクトとの貫通量を取得
        AABB pAABB = m_playerAABB;
        pAABB += Vector3{ m_velocity.translate.x, 0.0f, m_velocity.translate.z };
        m_wallPenetration = CollisionManager::GetInstance()->GetPenetrationForAABB(pAABB, true);

        // 現状ウォールランの移動量が斜めになることは無いので貫通量と視点方向から移動方向を算出
        Transform affine = Transform::Default;
        affine.rotate.y = m_cameraTransform.rotate.y;
        Matrix4x4 cameraMatrix = MakeAffineMatrix(affine);
        Vector3 cameraDirection = TransformNormal({ 0.0f, 0.0f, 1.0f }, cameraMatrix);
        // 貫通量は0か0以外であることが分かればよい

        Vector3 wallpenetration = Vector3::Zero;
        if (m_wallPenetration.x != 0.0f)
        {
            wallpenetration.x = 1.0f;
        }
        if (m_wallPenetration.z != 0.0f)
        {
            wallpenetration.z = 1.0f;
        }
        m_wallRunDirection.x = Sign(wallpenetration.z) * Sign(cameraDirection.x);
        m_wallRunDirection.z = Sign(wallpenetration.x) * Sign(cameraDirection.z);

        // ウォールラン用のオブジェクトとほぼ垂直の視点の場合直前の入力を使って移動方向を算出する
        if (std::fabs(cameraDirection.z) < 0.1f)
        {
            Vector3 walkDirection = TransformNormal({ m_command.move.x, 0.0f, m_command.move.y }, cameraMatrix);

            m_wallRunDirection.x = Sign(wallpenetration.z) * Sign(walkDirection.z);
            m_wallRunDirection.z = Sign(wallpenetration.x) * Sign(walkDirection.x);
        }
        m_wallRunDirection *= m_runSpeed;
        m_isStartWallRun = true;
    }

    m_moveAmount = { fabs(m_wallRunDirection.x), fabs(m_wallRunDirection.z) };;


    // 現在の速度を計算
    m_playerSpeed = max(fabs(m_moveAmount.x), fabs(m_moveAmount.y));
}

void Player::Sliding()
{
    m_moveSpeedPre = m_moveSpeed;

    float speed = m_crounchSpeed;

    // プレイヤーの歩行状態に応じて移動速度を設定

    // 移動速度の補間(速度が上がる時には即座に反映、速度が下がる時には時間をかけて反映)
    if (speed < m_moveSpeedPre && !m_isSpeedDecel)
    {
        m_speedBefore = m_moveSpeedPre;
        m_speedAfter = speed;
        m_isSpeedDecel = true;
    }
    if (m_moveSpeed < speed)
    {
        m_moveSpeed = speed;
        m_isSpeedDecel = false;
        m_moveSpeedTimer = 0.0f;
    }

    // 移動速度減速中のタイマー処理
    if (m_isSpeedDecel)
    {
        m_moveSpeedTimer += m_delta / m_speedDecelTime;
        float t = clamp(m_moveSpeedTimer, 0.0f, 1.0f);
        m_moveSpeed = Lerp(m_speedBefore, m_speedAfter, t);
    }
    // 減速が完了したらフラグとタイマーをリセット
    if (m_moveSpeedTimer >= 1.0f)
    {
        m_isSpeedDecel = false;
        m_moveSpeedTimer = 0.0f;
    }

    Vector2 fabsMoveInput = { fabs(m_command.move.x), fabs(m_command.move.y) };

    if (m_command.move.x == 0.0f)
    {
        fabsMoveInput.x = 1.0f;
    }
    if (fabsMoveInput.y == 0.0f)
    {
        fabsMoveInput.y = 1.0f;
    }

    // 移動量のクランプ
    m_moveAmount.x = clamp(m_moveAmount.x, -m_moveSpeed * fabsMoveInput.x, m_moveSpeed * fabsMoveInput.x);
    m_moveAmount.y = clamp(m_moveAmount.y, -m_moveSpeed * fabsMoveInput.y, m_moveSpeed * fabsMoveInput.y);

    m_decelMoveSpeed = m_runSpeed;

    // 一定速度で減速して0.0fにする
    m_moveAmount.x -= Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed);
    // オーバーシュート防止
    if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
        m_moveAmount.x = 0.0f;
    }
    m_moveAmount.y -= Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed);
    // オーバーシュート防止
    if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
        m_moveAmount.y = 0.0f;
    }

    // 斜め移動補正（正規化）
    float len = Length(m_moveAmount);
    if (len > 1.0f) {
        m_moveAmount /= len;
    }

    // MoveDirectionをm_cameraTransform.rotateの向きに等速で合わせる
    if (m_moveDirection.y != m_cameraTransform.rotate.y) {
        // Y軸回転の差分を計算
        float diff = m_cameraTransform.rotate.y - m_moveDirection.y;
        // 地上と空中で回転速度を変える
        float turnFactor = m_turnControlFactor;
        if (!m_onGround)
        {
            turnFactor = m_airControlFactor;
        }
        // 角度の差分を-180度から180度の範囲に収める
        float adjust = Sign(diff) * m_moveSpeed * (m_delta / turnFactor);
        // 差分が調整量より小さい場合は直接合わせる
        if (abs(diff) < abs(adjust)) { // 調整量より差分が小さい場合
            m_moveDirection.y = m_cameraTransform.rotate.y;
        }
        else { // 調整量分だけ移動方向を回転させる
            m_moveDirection.y += adjust;
        }
    }

    // 現在の速度を計算
    m_playerSpeed = max(fabs(m_moveAmount.x), fabs(m_moveAmount.y));
}

void Player::StartClimbing()
{
    // よじ登りの最終地点などの計算
    float height = m_playerAABB.max.y - m_playerAABB.min.y;

    AABB pAABB = m_playerAABB + m_velocity.translate;
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

    AABB colObj = CollisionManager::GetInstance()->GetObjectForCollisionDirection(pAABB, cameraDir);

    Vector3 rePairPosition = Vector3::Zero;
    if (cameraDir.x != 0.0f)
    {
        if (cameraDir.x > 0.0f)
        {
            rePairPosition.x = m_transform.translate.x - colObj.min.x;
        }
        if (cameraDir.x < 0.0f)
        {
            rePairPosition.x = m_transform.translate.x - colObj.max.x;
        }
    }
    if (cameraDir.z != 0.0f)
    {
        if (cameraDir.z > 0.0f)
        {
            rePairPosition.z = m_transform.translate.z - colObj.min.z;
        }
        if (cameraDir.z < 0.0f)
        {
            rePairPosition.z = m_transform.translate.z - colObj.max.z;
        }
    }

    float groundObjectDist = CollisionManager::GetInstance()->GetHeightToTopForAABB(pAABB);
    rePairPosition.y = groundObjectDist;

    m_climbingStartTranslate = m_transform.translate;
    m_climbingEndTranslate = m_transform.translate - rePairPosition;
    m_climbingTopTranslate = m_climbingStartTranslate + ((m_climbingEndTranslate - m_climbingStartTranslate) * 0.35f);
    m_climbingTopTranslate.y = max(m_climbingStartTranslate.y, m_climbingEndTranslate.y) + max(height * 0.15f, 0.35f);

    m_climbingTimer = 0.0f;
    m_climbingStep = 0;
    m_isClimbingMotion = true;
    m_onGround = true;
    m_state = WalkState::Idle;
    m_wallRunning = false;
    m_isStartWallRun = false;
    m_wallPenetration = Vector3::Zero;
    m_velocity.translate = Vector3::Zero;
    m_moveAmount = Vector2::Zero;
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
        climbingPosition = EaseInOut(m_climbingStartTranslate, m_climbingTopTranslate, t);
    }
    else if (m_climbingStep == 1)
    {
        climbingPosition = Lerp(m_climbingTopTranslate, m_climbingEndTranslate, t);
    }

    Vector3 moveDelta = climbingPosition - m_transform.translate;
    m_transform.translate = climbingPosition;
    m_playerAABB += moveDelta;
    m_velocity.translate = Vector3::Zero;
    m_moveAmount = Vector2::Zero;
    m_playerSpeed = 0.0f;
    m_onGround = true;

    if (t >= 1.0f)
    {
        if (m_climbingStep == 1)
        {
            m_isClimbingMotion = false;
            m_walkState = PlayerWalkState::Walk;
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
    switch (m_walkState)
    {
    case Player::PlayerWalkState::WallRun:
        m_velocity.translate.y = jumpStartVelocity;
        break;
    case Player::PlayerWalkState::Sliding:
        break;
    default:
        if (m_onGround) // 設置状態のジャンプ処理
        {
            m_velocity.translate.y = jumpStartVelocity;
        }
        break;
    }
}

void Player::ApplyCollision()
{
    CollisionManager::GetInstance()->UpdateCollisionTarget(m_playerAABB, "Player");
    CollisionManager::GetInstance()->Update("Player");
    m_transform.translate += m_velocity.translate;
    Vector3 penetration = CollisionManager::GetInstance()->GetPenetration();
    m_transform.translate -= penetration;
    m_playerAABB -= penetration;
}

void Player::ApplyGravity()
{
    // 重力の適用処理の実装
    if (!m_onGround)
    {
        m_fallVelocity = m_velocity.translate.y + m_gravity.y * m_delta;
        float groundDist = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
        // 落下速度の上限
        //m_velocity.translate.y = m_velocity.translate.y + m_fallVelocity * m_delta;
        m_velocity.translate.y += m_gravity.y * m_delta;
        m_velocity.translate.y = max(m_fallVelocity, -groundDist);
        // 壁走り中には落下速度を固定する
        if (m_wallRunning && m_velocity.translate.y < 0.0f)
        {
            m_velocity.translate.y = m_wallRunGravity * m_delta;
        }
    }
}

void Player::UpdateVelocity()
{
    if (!m_wallRunning)
    {
        // 移動方向の計算
        Transform dir = Transform::Default;
        dir.rotate.y = m_moveDirection.y;
        Matrix4x4 rotMat = MakeAffineMatrix(dir);
        Vector3 moveDir = TransformNormal({ m_moveAmount.x, 0.0f, m_moveAmount.y }, rotMat);
        m_velocity.translate = { moveDir.x * m_delta, m_velocity.translate.y, moveDir.z * m_delta };
        m_playerAABB += m_velocity.translate;
    }
    else
    {
        m_velocity.translate = { m_wallRunDirection.x * m_delta, m_velocity.translate.y, m_wallRunDirection.z * m_delta };
        m_playerAABB += m_velocity.translate;
    }


}

void Player::UpdateCameraParent() {
    // カメラのParent設定処理の実装
    m_pCamera->SetParent(m_pModel->GetWorldMatrix());
}

void Player::ApplyCameraEffect()
{
    // Fov変更処理の実装
    m_fov = m_pCamera->GetfovY();
    m_fovPre = m_fov;

    // しゃがみ用にカメラを下げる
    m_cameraTransform.translate.y = m_cameraHeight + m_crouchHeight;

    // 移動速度がダッシュ速度ならFovを広げる

    if (m_playerSpeed == m_runSpeed)
    {
        m_fovChangeTimer = 0.0f;
        m_fovBefore = m_fov;
        m_fovAfter = m_fovRun;
    }
    else
    {
        m_fovChangeTimer = 0.0f;
        m_fovBefore = m_fov;
        m_fovAfter = m_fovDefault;
    }

    m_fovChangeTimer += GameTime::GetInstance()->GetDeltaTime() / m_fovChangeTime;
    m_fovChangeTimer = clamp(m_fovChangeTimer, 0.0f, 1.0f);
    m_fov = Lerp(m_fovBefore, m_fovAfter, m_fovChangeTimer);
    // 計算結果をカメラにセット
    m_pCamera->SetFovY(m_fov);


    // ウォールダッシュをしたときのカメラの傾き
    if (m_wallRunning)
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
            float wallRunPenetration = 1.0f;
            if (m_wallPenetration.z < 0.0f || m_wallPenetration.x)
            {
                wallRunPenetration = -1.0f;
            }
            // 回転後角度を代入 回転後の角度は移動方向、壁がプレイヤーから左右どちらにあるかによって変わるのでそれも考慮する
            // 移動方向に応じての回転を入れいる
            Vector3 cameraDirection = m_pCamera->GetDirection();
            if (m_wallRunDirection.x != 0.0f)
            {
                m_wallRunRotateAfter = m_wallRunRotateAngle * signWallRunDirection.x * Sign(cameraDirection.z);
            }
            if (m_wallRunDirection.z != 0.0f)
            {
                m_wallRunRotateAfter = m_wallRunRotateAngle * signWallRunDirection.z * Sign(cameraDirection.x);
            }

        }

        m_wallRunTimer += m_delta / m_wallRunTime;
    }
    else
    {
        m_completeGetRotateInfo = false;
        m_wallRunTimer -= m_delta / m_wallRunTime;
    }
    m_wallRunTimer = clamp(m_wallRunTimer, 0.0f, 1.0f);

    m_cameraTransform.rotate.z = Lerp(0.0f, m_wallRunRotateAfter, m_wallRunTimer);

}

void Player::UpdateModelAnimation() {
    std::string animName = "DefaultAnimation";

    switch (m_walkState)
    {
    case Player::PlayerWalkState::Walk:
        if (m_command.move.y < 0.0f)
        {
            animName = "walk_back";
        }
        else
        {
            animName = "walk";
        }
        break;
    case Player::PlayerWalkState::Run:
        animName = "dash";
        break;
    case Player::PlayerWalkState::Crouch:
        if (m_state == WalkState::Move)
        {
            animName = "sneak";
        }
        else
        {
            animName = "crouch";
        }
        break;
    case Player::PlayerWalkState::WallRun:
        animName = "wall_run";
        break;
    case Player::PlayerWalkState::Sliding:
        animName = "slide";
        break;
    case Player::PlayerWalkState::Climbing:
        animName = "climb";
        break;
    default:
        break;
    }

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
        m_cameraTransform.translate = Vector3::Zero;
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

    ImGui::Text("PlayerState: %s", stateItems[stateIndex]);

    static const char* walkItems[] = { "Walk", "Run", "Crounch", "WallDash", "Sliding" };
    int walkIndex = static_cast<int>(m_walkState);

    ImGui::Text("WalkState: %s", walkItems[walkIndex]);

    ImGui::Checkbox("OnGround", &m_onGround);

    ImGui::Checkbox("WallRun", &m_wallRunning);

    ImGui::Checkbox("CanClimbing", &m_canClimbing);

    ImGui::DragFloat("CrouchHeihgt", &m_crouchHeight, 0.0f);
    ImGui::DragFloat("CrouchHeightOffset", &m_crouchHeightOffset, 0.01f);

    ImGui::DragFloat3("WallRunObjectMin", &m_wallRunningObjectAABB.min.x);
    ImGui::DragFloat3("WallRunObjectMax", &m_wallRunningObjectAABB.max.x);

    ImGui::Separator();

    // --- Movement ---
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::Text("Move Input: (%f, %f)", m_command.move.x, m_command.move.y);
        ImGui::Text("Move Amount: (%f, %f)", m_moveAmount.x, m_moveAmount.y);

        ImGui::Text("Gravity: (%.2f, %.2f, %.2f)",
            m_gravity.x, m_gravity.y, m_gravity.z);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 0.1f, 0.0f, 100.0f);

        ImGui::DragFloat("Jump Height", &m_jumpHeight, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat3("Gravity", &m_gravity.x, 0.1f);

        float groundDist = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
        ImGui::Text("Ground Distance: %.2f", groundDist);

        AABB pAABB = m_playerAABB + m_velocity.translate;
        groundDist = CollisionManager::GetInstance()->GetGroundDistanceForAABB(pAABB);
        ImGui::Text("Ground Object Distance: %.2f", groundDist);
        // 貫通量
        Vector3 penetration = CollisionManager::GetInstance()->GetPenetration();
        ImGui::Text("Penetration: (%.2f, %.2f, %.2f)",
            penetration.x, penetration.y, penetration.z);

        AABB aabb = m_playerAABB + Vector3{ m_velocity.translate.x, 0.0f, m_velocity.translate.z };
        penetration = CollisionManager::GetInstance()->GetPenetrationForAABB(aabb, true);
        ImGui::Text("WallRun Penetration: (%.2f, %.2f, %.2f)", penetration.x, penetration.y, penetration.z);

        // Transform
        if (ImGui::TreeNode("Transform")) {
            ImGui::DragFloat3("##Translate", &m_transform.translate.x, 0.1f);
            ImGui::DragFloat3("##Rotate", &m_transform.rotate.x, 0.1f);
            ImGui::Text("Scale:(%.2f, %.2f, %.2f)",
                m_transform.scale.x, m_transform.scale.y, m_transform.scale.z);
            ImGui::TreePop();
        }

        // Velocity
        if (ImGui::TreeNode("Velocity")) {
            ImGui::DragFloat3("##Vel Translate", &m_velocity.translate.x, 0.1f);
            ImGui::DragFloat3("##Vel Rotate", &m_velocity.rotate.x, 0.1f);
            float moveSpeed = Length({ m_velocity.translate.x, 0.0f, m_velocity.translate.z });
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
            // 加速時間
            ImGui::DragFloat("Max Speed Time", &m_maxSpeedTime, 0.01f);
            // 減速時間
            ImGui::DragFloat("Decel Time", &m_decelTime, 0.01f);
            ImGui::TreePop();
        }

        if (ImGui::CollapsingHeader("GamePad")) {
            // 左スティック入力
            Vector2 leftStick = Input::GetInstance()->GetLeftJoyStickPos2(0.0f) / 1000.0f;
            ImGui::Text("Left Stick: (%.2f, %.2f)",
                leftStick.x, leftStick.y);
            // 右スティック入力
            Vector3 rightStick = Input::GetInstance()->GetRightJoyStickPos3(0.0f);
            ImGui::Text("Right Stick: (%.2f, %.2f, %.2f)",
                rightStick.x, rightStick.y, rightStick.z);

        }
    }

    ImGui::Separator();

    // --- Camera ---
    if (ImGui::CollapsingHeader("Camera")) {

        ImGui::DragFloat("Camera Height", &m_cameraHeight, 0.01f);

        if (ImGui::TreeNode("Camera Transform")) {
            ImGui::Text("Pos:  (%.2f, %.2f, %.2f)",
                m_cameraTransform.translate.x, m_cameraTransform.translate.y, m_cameraTransform.translate.z);
            ImGui::Text("Rot:  (%.2f, %.2f, %.2f)",
                m_cameraTransform.rotate.x, m_cameraTransform.rotate.y, m_cameraTransform.rotate.z);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Camera Velocity")) {
            ImGui::Text("Vel Pos: (%.2f, %.2f, %.2f)",
                m_cameraVelocity.translate.x, m_cameraVelocity.translate.y, m_cameraVelocity.translate.z);
            ImGui::Text("Vel Rot: (%.2f, %.2f, %.2f)",
                m_cameraVelocity.rotate.x, m_cameraVelocity.rotate.y, m_cameraVelocity.rotate.z);
            ImGui::TreePop();
        }

        ImGui::SliderFloat("FOV", &m_fov, 30.0f, 120.0f);
    }

    ImGui::End();
}

void Player::MovementGodMode()
{
    // 視点移動
    m_cameraTransform.rotate.x += Input::GetInstance()->GetMouseVel2().y * 0.001f;
    m_cameraTransform.rotate.y += Input::GetInstance()->GetMouseVel2().x * 0.001f;

    // 移動処理
    float delta = GameTime::GetInstance()->GetDeltaTime();
    Vector3 moveDir = Vector3::Zero;
    // キーボード移動
    moveDir.z += -Input::GetInstance()->PushKeyInt(DIK_S);
    moveDir.z += Input::GetInstance()->PushKeyInt(DIK_W);
    moveDir.x += -Input::GetInstance()->PushKeyInt(DIK_A);
    moveDir.x += Input::GetInstance()->PushKeyInt(DIK_D);
    m_cameraTransform.translate.y += Input::GetInstance()->PushKeyInt(DIK_SPACE);
    m_cameraTransform.translate.y += -Input::GetInstance()->PushKeyInt(DIK_LCONTROL);

    // 視点方向に移動
    Transform dir = Transform::Default;
    dir.rotate = m_cameraTransform.rotate;
    Vector3 moveVelocity = TransformNormal(moveDir, MakeAffineMatrix(dir));
    m_cameraVelocity.translate = moveVelocity * delta * 20.0f;

    m_cameraTransform.translate += m_cameraVelocity.translate;
}

#endif // !NDEBUG
