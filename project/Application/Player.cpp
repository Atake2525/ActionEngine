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

using namespace std;


// 斜め移動の速度補間
constexpr float INV_SQRT2 = 0.70710678f; // 1 / sqrt(2)

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

    // プレイヤーモデルの初期化
    m_pModel = make_unique<Object3d>();
    m_pModel->Initialize();
    m_pModel->SetModel("Resources/Model/obj/Player", "PlayerCollision.obj", true);
    m_pModel->SetTransform(m_transform);
    m_playerAABB = m_pModel->GetAABB();
    m_playerAABB += m_transform.translate;
    CollisionManager::GetInstance()->AddCollisionTarget(m_playerAABB, "Player");

    // カメラの高さをモデルの高さに合わせて調整 (ちょっとだけ低くする)
    m_cameraTransform.translate.y = m_playerAABB.max.y - m_transform.translate.y - 0.05f;

    // コントロールモードの初期設定
    if (Input::GetInstance()->IsConnectedController())
    {
        m_controlMode = ControlMode::Gamepad;
    }
    else
    {
        m_controlMode = ControlMode::KeyboardMouse;
    }

    input = Input::GetInstance();

    // デバッグ用の初期設定
#ifndef NDEBUG
    Input::GetInstance()->ShowMouseCursor(true);
#else
    Input::GetInstance()->ShowMouseCursor(false);
#endif // !NDEBUG

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
        //
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
    // 
    ApplyCollision();
    // 重力の適用
    ApplyGravity();
#endif // !NDEBUG


    //m_transform.translate += m_velocity.translate;

    m_transform.rotate += m_velocity.rotate;
    m_pModel->SetTransform(m_transform);

    if (!m_isFirstInput)
    {
        m_pModel->SetTransform(m_firstTransform);
    }

    m_pModel->Update();
    //m_pCamera->SetTranslate(m_transform.translate);
    m_pCamera->SetTransform(m_cameraTransform);
    ApplyCameraEffect();
    UpdateCameraParent();

}

void Player::Draw()
{
    m_pModel->Draw();
}

void Player::UpdateState()
{
    // 前回の状態を保存
    m_statePre = m_state;
    m_walkStatePre = m_walkState;

    // 入力が入ったら最初の入力フラグを立てる
    if (m_moveInput.x != 0.0f || m_moveInput.y != 0.0f)
    {
        m_isFirstInput = true;
    }
    
    if (m_onGround)
    {
        int statePoint = Sign((Sign(m_moveInput.x) * m_moveInput.x) + (Sign(m_moveInput.y) * m_moveInput.y));
        m_state = static_cast<PlayerState>(statePoint);
    }
    
    m_walkState = PlayerWalkState::Walk;

    // 空中判定(地面からの距離が一定以上離れているか)
    float groundDistance = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
    if (groundDistance > 0.2f)
    {
        m_onGround = false;
        m_state = PlayerState::Falling;
    }

    // コントロールモードに応じた入力処理
    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse: // キーボード・マウス
        // ダッシュ入力
        if (input->PushKeyInt(DIK_LSHIFT))
        {
            m_walkState = PlayerWalkState::Run;
        }
        // しゃがみ入力
        if (input->PushKeyInt(DIK_LCONTROL))
        {
            m_walkState = PlayerWalkState::Crounch;
        }

        break;
    case Player::ControlMode::Gamepad: // ゲームパッド
        // ダッシュ入力
        if (Input::GetInstance()->PushButton(Controller::LeftStick))
        {
            m_walkState = PlayerWalkState::Run;
        }

        // しゃがみ入力
        if (Input::GetInstance()->PushButton(Controller::RightStick))
        {
            m_walkState = PlayerWalkState::Crounch;
        }

        break;
    }

    UpdateParkourState();
}

void Player::UpdateParkourState()
{
    // 壁走りが可能かを調べる
    if (m_enableParkour)
    {
        switch (m_state)
        {
        case Player::PlayerState::Idle:
            break;
        case Player::PlayerState::Move:
            // 地上移動中のしゃがみ入力によるスライディング
            if (m_walkState == PlayerWalkState::Crounch) {
                // パルクールが有効で移動量が一定以上の場合
                float moveSpeed = Length({ m_velocity.translate.x, 0.0f, m_velocity.translate.z }) / m_delta;
                if (m_enableParkour && moveSpeed > m_walkSpeed)
                {
                    // スライディング入力
                    m_walkState = PlayerWalkState::Sliding;
                }
            }
            break;
        case Player::PlayerState::Falling:
            // 落下中かつ一定以上のサイズのオブジェクトに衝突している時に壁走り
            if (CollisionManager::GetInstance()->IsCollisionObjectForAABB(m_playerAABB) && m_velocity.translate.y < 0.0f)
            {

            }
            break;
        }
    }
}

void Player::HandleInput()
{
    // 移動入力のリセット
    m_moveInput = Vector2::Zero;
    m_jumpInput = 0.0f;

    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:
        // キー入力による移動
        m_moveInput.y += -Input::GetInstance()->PushKeyInt(DIK_W);
        m_moveInput.y += Input::GetInstance()->PushKeyInt(DIK_S);
        m_moveInput.x += -Input::GetInstance()->PushKeyInt(DIK_A);
        m_moveInput.x += Input::GetInstance()->PushKeyInt(DIK_D);
        m_jumpInput += Input::GetInstance()->PushKeyInt(DIK_SPACE);

        m_enableParkour = Input::GetInstance()->PressMouse(1);

        break;
    case Player::ControlMode::Gamepad:
        // ジョイスティック入力による移動
        m_moveInput = input->GetJoyStickVelocity();
        m_jumpInput += Input::GetInstance()->PushButton(Controller::A);

        break;
    }


}

void Player::Rotate() {
    Vector3 rotate = Vector3::Zero;
    // カメラ処理の実装
    // マウスの移動量に基づいてカメラの回転を更新    演出実装時に加速度を付けるなどの調整を行う予定
    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:
        rotate = Input::GetInstance()->GetMouseVel3() * 0.001f;
        break;
    case Player::ControlMode::Gamepad:
        rotate = Input::GetInstance()->GetRightJoyStickPos3(0.0f) * 0.00005f;
        break;
    }
    /*m_transform.rotate.x += rotate.y;
    m_transform.rotate.y += rotate.x;*/

    m_cameraTransform.rotate.x += rotate.y;
    m_cameraTransform.rotate.y += rotate.x;


}

void Player::Move() {

    Walk();
    Jump();

    // MoveDirectionをm_cameraTransform.rotateの向きに等速で合わせる
    if (m_moveDirection.y != m_cameraTransform.rotate.y) {
        // Y軸回転の差分を計算
        float diff = m_cameraTransform.rotate.y - m_moveDirection.y;
        // 地上と空中で回転速度を変える
        float turnFactor = m_turnControlFactor;
        if (m_state == PlayerState::Falling)
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
    //// 移動方向の計算
    Transform dir = Transform::Default;
    dir.rotate.y = m_moveDirection.y;
    //dir.rotate.y = m_cameraTransform.rotate.y;
    Matrix4x4 rotMat = MakeAffineMatrix(dir);
    Vector3 moveDir = TransformNormal({ m_moveAmount.x, 0.0f, -m_moveAmount.y }, rotMat);

    m_velocity.translate = { moveDir.x * m_delta, m_velocity.translate.y, moveDir.z * m_delta };
    m_playerAABB += m_velocity.translate;

    //// 現在の移動方向（地上処理で作った moveDir）
    //Vector3 v = { moveDir.x, 0.0f, moveDir.z };
    //float sp = Length(v);

    //if (sp > 0.0001f)
    //    v = v / sp; // 正規化
    //else
    //    v = { 0,0,0 };

    //// 入力方向（カメラ基準）
    //Vector3 desiredDir = TransformNormal(
    //    { m_moveInput.x, 0.0f, -m_moveInput.y }, rotMat
    //);
    //desiredDir = Normalize(desiredDir);

    //// 補間係数（0〜1）
    //float t = m_airControlFactor; // 例：0.05〜0.2

    //// 方向だけ補間
    //Vector3 newDir = Normalize(v * (1.0f - t) + desiredDir * t);

    //// 速度の大きさは維持
    //Vector3 newVelocity = newDir * sp;

    //// 実際の速度に反映
    //m_velocity.translate.x = newVelocity.x * m_delta;
    //m_velocity.translate.z = newVelocity.z * m_delta;
}

void Player::Walk()
{
#pragma region 移動(歩行)
    // m_moveInput(入力)に基づいてプレイヤーの移動方向を決定
    if (m_moveInput.x != 0.0f || m_moveInput.y != 0.0f)
    {
        // 入力中は移動量を上げる
        m_moveAmount.x += Sign(m_moveInput.x) * (m_delta / m_maxSpeedTime * m_walkSpeed);
        m_moveAmount.y += Sign(m_moveInput.y) * (m_delta / m_maxSpeedTime * m_walkSpeed);
    }

    m_moveSpeedPre = m_moveSpeed;

    float speed = 0.0f;

    // プレイヤーの歩行状態に応じて移動速度を設定
    switch (m_walkState)
    {
    case Player::PlayerWalkState::Walk:
        speed = m_walkSpeed;
        break;
    case Player::PlayerWalkState::Run:
        speed = m_runSpeed;
        break;
    case Player::PlayerWalkState::Crounch:
        speed = m_crounchSpeed;
        break;
    }

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

    Vector2 fabsMoveInput = { fabs(m_moveInput.x), fabs(m_moveInput.y) };

    if (m_moveInput.x == 0.0f)
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

    // 入力が無かったら一定速度で減速して0.0fにする
    // 減速は加速よりも早くする
    if (m_moveInput.x == 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (m_moveInput.y == 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (m_delta / m_decelTime * m_decelMoveSpeed))) {
            m_moveAmount.y = 0.0f;
        }
    }
    // 反対入力があったら減速速度を上げる
    if (Sign(m_moveInput.x) != Sign(m_moveAmount.x) && m_moveInput.x != 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (m_delta / (m_decelTime / 2.0f) * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (m_delta / (m_decelTime / 2.0f) * m_walkSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (Sign(m_moveInput.y) != Sign(m_moveAmount.y) && m_moveInput.y != 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (m_delta / (m_decelTime / 2.0f) * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (m_delta / (m_decelTime / 2.0f) * m_walkSpeed))) {
            m_moveAmount.y = 0.0f;
        }
    }

    // 斜め移動補正（正規化）
    float len = Length(m_moveAmount);
    if (len > 1.0f) {
        m_moveAmount /= len;
    }
#pragma endregion
}

void Player::Jump()
{
    // ジャンプ入力があったらジャンプ処理
    if (m_jumpInput > 0.0f && m_state != PlayerState::Falling)
    {
        // ジャンプ量(現在のY座標+m_jumpForce)の値を到達地点として設定
        m_velocity.translate.y = sqrtf(2.0f * -m_gravity.y * m_jumpForce);

        //m_velocity.translate.y = (m_jumpForce * m_jumpForce) / (2.0f * -m_gravity.y);
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
    if (m_state == PlayerState::Falling)
    {
        m_fallVelocity = m_velocity.translate.y + m_gravity.y * m_delta;
        float groundDist = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
        // 落下速度の上限
        //m_velocity.translate.y = m_velocity.translate.y + m_fallVelocity * m_delta;
        m_velocity.translate.y += m_gravity.y * m_delta;
        m_velocity.translate.y = max(m_fallVelocity, -groundDist);
        // 壁走り中には落下速度を固定する
        if (m_wallDash && m_velocity.translate.y < 0.0f)
        {
            m_velocity.translate.y = m_wallDashGravity;
        }
        if (groundDist <= 0.01f)
        {
            m_onGround = true;
            m_fallVelocity = 0.0f;
            m_velocity.translate.y = 0.0f;
            // 
            m_transform.translate.y -= groundDist;
        }
    }
}

void Player::UpdateCameraParent() {
    // カメラのParent設定処理の実装
    //m_pCamera->SetRotateParent(m_pModel->GetWorldMatrix());
    m_pCamera->SetParent(m_pModel->GetWorldMatrix());
    //m_pModel->SetParent(m_pCamera->GetWorldMatrix());
}

#ifndef NDEBUG
void Player::ApplyCameraEffect()
{
    // Fov変更処理の実装
    m_fovPre = m_fov;
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




}
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


    ImGui::Separator();

    // --- Movement ---
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::Text("Move Input: (%f, %f)", m_moveInput.x, m_moveInput.y);
        ImGui::Text("Move Amount: (%f, %f)", m_moveAmount.x, m_moveAmount.y);

        ImGui::Text("Gravity: (%.2f, %.2f, %.2f)",
            m_gravity.x, m_gravity.y, m_gravity.z);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 0.1f, 0.0f, 100.0f);

        // ジャンプ力
        ImGui::DragFloat("Jump Force", &m_jumpForce, 0.1f);
        ImGui::DragFloat3("Gravity", &m_gravity.x, 0.1f);

        float groundDist = CollisionManager::GetInstance()->GetMaxGroundDistanceForAABB(m_playerAABB);
        ImGui::Text("Ground Distance: %.2f", groundDist);

        // 貫通量
        Vector3 penetration = CollisionManager::GetInstance()->GetPenetration();
        ImGui::Text("Penetration: (%.2f, %.2f, %.2f)",
            penetration.x, penetration.y, penetration.z);

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
            ImGui::Text("Speed : %f", m_playerSpeed);
            // 歩行速度
            ImGui::DragFloat("Walk Speed", &m_walkSpeed, 0.1f);
            ImGui::DragFloat("Run Speed", &m_runSpeed, 0.1f);
            ImGui::DragFloat("Crounch Speed", &m_crounchSpeed, 0.1f);
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
