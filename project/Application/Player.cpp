#include "Player.h"
#include "JsonLoader.h"
#include "Camera.h"
#include "Input.h"
#include "ImGuiManager.h"

using namespace std;

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
    m_pModel->SetModel("Resources/Model/obj/Player", "Player.obj", true);
    m_pModel->SetTransform(m_transform);
}

void Player::Update()
{
    m_pModel->SetTransform(m_transform);
    m_pModel->Update();
    m_pCamera->SetTranslate(m_transform.translate);
    m_pCamera->Update();

    if (!ImGui::Begin("Player Debug")) {
        ImGui::End();
        return;
    }

    // --- Control Mode ---
    ImGui::Text("Control Mode: %s",
        m_controlMode == ControlMode::KeyboardMouse ? "Keyboard & Mouse" : "Gamepad");

    // --- Player State ---
    ImGui::Text("State: %d", static_cast<int>(m_state));
    ImGui::Text("Prev State: %d", static_cast<int>(m_statePre));

    ImGui::Separator();

    // --- Movement ---
    if (ImGui::CollapsingHeader("Movement")) {
        ImGui::Text("Move Input: (%.2f, %.2f)", m_moveInput.x, m_moveInput.y);

        ImGui::Text("Gravity: (%.2f, %.2f, %.2f)",
            m_gravity.x, m_gravity.y, m_gravity.z);

        // Transform
        if (ImGui::TreeNode("Transform")) {
            ImGui::Text("Pos:  (%.2f, %.2f, %.2f)",
                m_transform.translate.x, m_transform.translate.y, m_transform.translate.z);
            ImGui::Text("Rot:  (%.2f, %.2f, %.2f)",
                m_transform.rotate.x, m_transform.rotate.y, m_transform.rotate.z);
            ImGui::Text("Scale:(%.2f, %.2f, %.2f)",
                m_transform.scale.x, m_transform.scale.y, m_transform.scale.z);
            ImGui::TreePop();
        }

        // Velocity
        if (ImGui::TreeNode("Velocity")) {
            ImGui::Text("Vel Pos: (%.2f, %.2f, %.2f)",
                m_velocity.translate.x, m_velocity.translate.y, m_velocity.translate.z);
            ImGui::Text("Vel Rot: (%.2f, %.2f, %.2f)",
                m_velocity.rotate.x, m_velocity.rotate.y, m_velocity.rotate.z);
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

void Player::Draw()
{
    m_pModel->Draw();
}

void Player::UpdateState()
{
}

void Player::HandleInput()
{
    // 移動入力のリセット
    m_moveInput = Vector2::Zero;

    // 前回の状態を保存
    m_statePre = m_state;

    switch (m_controlMode)
    {
    case Player::ControlMode::KeyboardMouse:
        // キー入力による移動
        if (Input::GetInstance()->PushKey(DIK_W))
        {
            m_moveInput.y = -1.0f;
        }
        if (Input::GetInstance()->PushKey(DIK_S))
        {
            m_moveInput.y = 1.0f;
        }
        if (Input::GetInstance()->PushKey(DIK_A))
        {
            m_moveInput.x = -1.0f;
        }
        if (Input::GetInstance()->PushKey(DIK_D))
        {
            m_moveInput.x = 1.0f;
        }

        // 移動入力に基づく状態遷移
        if (m_moveInput.x != 0.0f || m_moveInput.y != 0.0f)
        {
            m_state = PlayerState::Walking;
        }
        else // 移動入力なし
        {
            m_state = PlayerState::Idle;
        }

        // ダッシュ入力
        if (Input::GetInstance()->PushKey(DIK_LSHIFT))
        {
            m_state = PlayerState::Running;
        }

        // ジャンプ入力
        if (Input::GetInstance()->PushKey(DIK_SPACE))
        {
            m_state = PlayerState::Jumping;
        }
        break;
    case Player::ControlMode::Gamepad:
        // ジョイスティック入力による移動
        m_moveInput = Input::GetInstance()->GetJoyStickVelocity(
            Input::GetInstance()->GetLeftJoyStickPos2(0.2f),
            Vector3{ 1.0f, 1.0f, 1.0f },
            true
        );
        // 移動入力に基づく状態遷移
        if (m_moveInput.x != 0.0f || m_moveInput.y != 0.0f)
        {
            m_state = PlayerState::Walking;
        }
        else // 移動入力なし
        {
            m_state = PlayerState::Idle;
        }

        // ダッシュ入力
        if (Input::GetInstance()->TriggerButton(Controller::LeftStick))
        {
            m_state = PlayerState::Running;
        }

        // ジャンプ入力
        if (Input::GetInstance()->TriggerButton(Controller::A))
        {
            m_state = PlayerState::Jumping;
        }
        break;
    }


}
