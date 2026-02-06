#include "Player.h"
#include "JsonLoader.h"
#include "Camera.h"
#include "Input.h"
#include "ImGuiManager.h"
#include "kMath.h"
#include "GameTime.h"

using namespace std;


// 斜め移動の速度補間
constexpr float INV_SQRT2 = 0.70710678f; // 1 / sqrt(2)

void Player::Initialize(Camera* camera, const std::string& jsonName)
{
    // カメラのセット
    m_pCamera = camera;

    // JsonDataからステージ情報を取得してプレイヤーの初期位置とゴールの位置を設定する処理
    if (JsonLoader::GetInstance()->CheckJsonLoaded(jsonName))
    {
        // スタート地点の取得
        vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(jsonName, "startpoint");
        // スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
        if (!data.empty())
        {
            m_transform = data[0].transform;
        }
    }

    // プレイヤーモデルの初期化
    m_pModel = make_unique<Object3d>();
    m_pModel->Initialize();
    m_pModel->SetModel("Resources/Model/obj/Player", "PlayerCollision.obj", true);
    m_pModel->SetTransform(m_transform);

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
#endif // !NDEBUG

    // 最終的な変更をTransformに反映
    m_transform.translate += m_velocity.translate;
    m_transform.rotate += m_velocity.rotate;
    m_pModel->SetTransform(m_transform);
    m_pModel->Update();
    //m_pCamera->SetTranslate(m_transform.translate);
    m_pCamera->SetTransform(m_cameraTransform); 
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

    float statePoint = Sign((Sign(m_moveInput.x) * m_moveInput.x) + (Sign(m_moveInput.y) * m_moveInput.y));

    m_state = static_cast<PlayerState>(statePoint);
    // 入力確認用の変数
    int state = 0;

    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:

        // 入力の確認 orを使って値の大きいものを優先させる
        state |= input->PushKeyInt(DIK_LSHIFT) * static_cast<int>(PlayerWalkState::Run);
        //state |= input->PushKeyInt(DIK_SPACE) * static_cast<int>(PlayerWalkState::Jumping); // ジャンプ処理
        state |= input->PushKeyInt(DIK_LCONTROL) * static_cast<int>(PlayerWalkState::Crounch);

        m_walkState = static_cast<PlayerWalkState>(state);

        break;
    case Player::ControlMode::Gamepad:
        // ダッシュ入力
        if (Input::GetInstance()->TriggerButton(Controller::LeftStick))
        {
            m_walkState = PlayerWalkState::Run;
        }

        // ジャンプ入力
        /*if (Input::GetInstance()->TriggerButton(Controller::A))
        {
            m_state = PlayerState::Jumping;
        }*/

        // しゃがみ入力
        if (Input::GetInstance()->TriggerButton(Controller::RightStick))
        {

        }

        break;
    }
}

void Player::HandleInput()
{
    // 移動入力のリセット
    m_moveInput = Vector2::Zero;

    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:
        // キー入力による移動
        m_moveInput.y += -Input::GetInstance()->PushKeyInt(DIK_W);
        m_moveInput.y += Input::GetInstance()->PushKeyInt(DIK_S);
        m_moveInput.x += -Input::GetInstance()->PushKeyInt(DIK_A);
        m_moveInput.x += Input::GetInstance()->PushKeyInt(DIK_D);

        break;
    case Player::ControlMode::Gamepad:
        // ジョイスティック入力による移動
        m_moveInput = Input::GetInstance()->GetJoyStickVelocity(
            Input::GetInstance()->GetLeftJoyStickPos2(0.2f),
            Vector3{ 1.0f, 1.0f, 1.0f },
            true
        );

        break;
    }


}

void Player::Rotate() {
    // カメラ処理の実装
    // マウスの移動量に基づいてカメラの回転を更新    演出実装時に加速度を付けるなどの調整を行う予定
    Vector3 rotate = Input::GetInstance()->GetMouseVel3() * 0.001f;
    /*m_transform.rotate.x += rotate.y;
    m_transform.rotate.y += rotate.x;*/

    m_cameraTransform.rotate.x += rotate.y;
    m_cameraTransform.rotate.y += rotate.x;


}

void Player::Move() {

    float delta = GameTime::GetInstance()->GetDeltaTime();

    // m_moveInput(入力)に基づいてプレイヤーの移動方向を決定
    m_moveAmount.x += Sign(m_moveInput.x) * (delta / m_maxSpeedTime * m_walkSpeed);
    m_moveAmount.y += Sign(m_moveInput.y) * (delta / m_maxSpeedTime * m_walkSpeed);

    // プレイヤーの歩行状態に応じて移動速度を設定
    switch (m_walkState)
    {
    case Player::PlayerWalkState::Walk:
        m_moveSpeed = m_walkSpeed;
        break;
    case Player::PlayerWalkState::Run:
        m_moveSpeed = m_runSpeed;
        break;
    case Player::PlayerWalkState::Crounch:
        m_moveSpeed = m_crounchSpeed;
        break;
    }

    // 移動量のクランプ
    m_moveAmount = Vector2::Clamp(m_moveAmount, -m_moveSpeed, m_moveSpeed);

    // 入力が無かったら一定速度で減速して0.0fにする
    // 減速は加速よりも早くする
    if (m_moveInput.x == 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (delta / m_decelTime * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (delta / m_decelTime * m_walkSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (m_moveInput.y == 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (delta / m_decelTime * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (delta / m_decelTime * m_walkSpeed))) {
            m_moveAmount.y = 0.0f;
        }
    }
    // 反対入力があったら減速速度を上げる
    if (Sign(m_moveInput.x) != Sign(m_moveAmount.x) && m_moveInput.x != 0.0f) {
        m_moveAmount.x -= Sign(m_moveAmount.x) * (delta / (m_decelTime / 2.0f) * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.x) != Sign(m_moveAmount.x - Sign(m_moveAmount.x) * (delta / (m_decelTime / 2.0f) * m_walkSpeed))) {
            m_moveAmount.x = 0.0f;
        }
    }
    if (Sign(m_moveInput.y) != Sign(m_moveAmount.y) && m_moveInput.y != 0.0f) {
        m_moveAmount.y -= Sign(m_moveAmount.y) * (delta / (m_decelTime / 2.0f) * m_walkSpeed);
        // オーバーシュート防止
        if (Sign(m_moveAmount.y) != Sign(m_moveAmount.y - Sign(m_moveAmount.y) * (delta / (m_decelTime / 2.0f) * m_walkSpeed))) {
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
        if (m_state == PlayerState::Falling)
        {
            turnFactor = m_airControlFactor;
        }
        // 角度の差分を-180度から180度の範囲に収める
        float adjust = Sign(diff) * m_moveSpeed * (delta / turnFactor);
        // 差分が調整量より小さい場合は直接合わせる
        if (abs(diff) < abs(adjust)) { // 調整量より差分が小さい場合
            m_moveDirection.y = m_cameraTransform.rotate.y;
        }
        else { // 調整量分だけ移動方向を回転させる
            m_moveDirection.y += adjust;
        }
    }

    // 移動方向の計算
    Transform dir = Transform::Default;
    dir.rotate.y = m_moveDirection.y;
    Vector3 moveDir = TransformNormal({ m_moveAmount.x, 0.0f, -m_moveAmount.y }, MakeAffineMatrix(dir));

    m_velocity.translate = moveDir * delta;
}

void Player::UpdateCameraParent() {
    // カメラのParent設定処理の実装
    //m_pCamera->SetRotateParent(m_pModel->GetWorldMatrix());
    m_pCamera->SetParent(m_pModel->GetWorldMatrix());
    //m_pModel->SetParent(m_pCamera->GetWorldMatrix());
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

    // --- Player State ---
    ImGui::Text("State: %d", static_cast<int>(m_state));
    ImGui::Text("Prev State: %d", static_cast<int>(m_statePre));

    static const char* stateItems[] = { "Idle", "Move", "Falling" };
    int stateIndex = static_cast<int>(m_state);

    ImGui::Text("PlayerState: %s", stateItems[stateIndex]);

    static const char* walkItems[] = { "Walk", "Run", "Crounch" };
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

        // Transform
        if (ImGui::TreeNode("Transform")) {
            //ImGui::Text("Pos:  (%.2f, %.2f, %.2f)",
                //m_transform.translate.x, m_transform.translate.y, m_transform.translate.z);
            ImGui::DragFloat3("##Translate", &m_transform.translate.x, 0.1f);
            /*ImGui::Text("Rot:  (%.2f, %.2f, %.2f)",
                m_transform.rotate.x, m_transform.rotate.y, m_transform.rotate.z);*/
            ImGui::DragFloat3("##Rotate", &m_transform.rotate.x, 0.1f);
            ImGui::Text("Scale:(%.2f, %.2f, %.2f)",
                m_transform.scale.x, m_transform.scale.y, m_transform.scale.z);
            ImGui::TreePop();
        }

        // Velocity
        if (ImGui::TreeNode("Velocity")) {
            /*ImGui::Text("Vel Translate: (%.2f, %.2f, %.2f)",
                m_velocity.translate.x, m_velocity.translate.y, m_velocity.translate.z);
            ImGui::Text("Vel Rotate: (%.2f, %.2f, %.2f)",
                m_velocity.rotate.x, m_velocity.rotate.y, m_velocity.rotate.z);*/
            ImGui::DragFloat3("##Vel Translate", &m_velocity.translate.x, 0.1f);
            ImGui::DragFloat3("##Vel Rotate", &m_velocity.rotate.x, 0.1f);
            ImGui::TreePop();
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

        ImGui::SliderFloat("FOV", &m_fovY, 30.0f, 120.0f);
        ImGui::Text("After FOV: %.2f", m_afterFovY);
        ImGui::Text("FOV Timer: %.2f / %.2f", m_fovTimer, m_fovTime);
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
