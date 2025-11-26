#include "ActionPlayer.h"
#include "JsonLoader.h"
#include "GameTime.h"
#include "Camera.h"
#include "ImGuiManager.h"

#ifndef NDEBUG	
	#include "Logger.h"
	using namespace Logger;
#endif // !NDEBUG


using namespace std;

ActionPlayer::ActionPlayer()
    : m_velocity({ 0.0f, 0.0f, 0.0f })
    , m_accelTime(0.4f)
    , m_moveSpeed(0.0f)
    , m_walkSpeed(0.1f)
    , m_dashSpeed(0.2f)
    , m_crouchSpeed(0.08f)
    , m_jumpForce(6.0f)
    , m_gravity(1.2f)
    , m_inputDirection({0.0f, 0.0f})
    , m_isGround(false)
    , m_isDash(false)
    , m_isCrouch(false)
    , m_isWallRun(false)
    , m_isWallJump(false)
    , m_wallNormal({0.0f, 0.0f, 0.0f})
    , m_groundDistance(0.0f)
    , m_wallDistance(0.0f)
{
}

Vector3 cameraRotate;
Transform playerTransform;

void ActionPlayer::Initialize(Camera* camera) {
    m_pPlayerModel = make_unique<Object3d>();
    m_pPlayerModel->Initialize();
    m_pPlayerModel->SetModel("Resources/Model/gltf/Player", "PlayerCollision.gltf");
    playerTransform = m_pPlayerModel->GetTransform();
    m_pCamera = camera;

    m_jumpForce = 6.0f;
    m_gravity = 2.0f;
    m_velocity = { 0.0f, 0.0f, 0.0f };

    m_pInput = Input::GetInstance();
}

void ActionPlayer::Update() {
	HandleMouseLock();
	//UpdateStates();

    HandleInput();                  // 入力取得（WASD, ジャンプ, しゃがみなど）

 //   if (m_isWallRunning) {
 //      // 壁走り中の移動処理
 //   }
 //   else {
 //       Move();           // 通常移動（地上・空中）
 //   }
    Move();
    Jump();

    /*if (m_isWallJumping) {
        if (m_isWallRunning) {
            WallJump();
        }
        else if (m_isGrounded) {
            Jump();
        }
    }*/

    ApplyGravity();       // 重力適用（空中時）

 //   // 実際の位置更新（RigidbodyやTransformに反映）
#ifndef NDEBUG
    Debug();
#endif // !NDEBUG

    playerTransform.translate += m_velocity;
    m_pPlayerModel->SetTranslate({ playerTransform.translate.x, playerTransform.translate.y, playerTransform.translate.z + 0.05f });
    m_pPlayerModel->Update();
    
    m_pCamera->SetParent(m_pPlayerModel->GetWorldMatrix());
    m_pCamera->SetRotate(cameraRotate);
    m_pCamera->Update();
}

void ActionPlayer::Draw() {
    m_pPlayerModel->Draw();
}

// WASDやジャンプなどの入力処理
void ActionPlayer::HandleInput() {
    m_inputDirection = { 0.0f, 0.0f };
    // 平行移動行動処理
    if (m_pInput->PushKey(DIK_W))
    {
        m_inputDirection.y = 1.0f;
    }
    if (m_pInput->PushKey(DIK_S))
    {
        m_inputDirection.y = -1.0f;
    }
    if (m_pInput->PushKey(DIK_A))
    {
        m_inputDirection.x = -1.0f;
    }
    if (m_pInput->PushKey(DIK_D))
    {
        m_inputDirection.x = 1.0f;
    }
   /* if (m_inputDirection.x != 0.0f && m_inputDirection.y != 0.0f)
    {
        m_inputDirection.x /= 2.0f;
        m_inputDirection.y /= 2.0f;
    }*/
    m_moveSpeed = m_walkSpeed;
    if (m_pInput->PushKey(DIK_LSHIFT))
    {
        m_moveSpeed = m_dashSpeed;
    }
    if (m_pInput->PushKey(DIK_LCONTROL))
    {
        m_moveSpeed = m_crouchSpeed;
    }

    // ダッシュ入力処理
    if (m_pInput->PushKey(DIK_LSHIFT))
    {
        m_isDash = true;
    }
    else
    {
        m_isDash = false;
    }

    // しゃがみ入力処理
    if (m_pInput->PushKey(DIK_LCONTROL))
    {
        m_isCrouch = true;
    }
    else
    {
        m_isCrouch = false;
    }
}

Vector3 moveVelocity = { 0.0f, 0.0f, 0.0f };
// 移動処理 (地上・空中)
void ActionPlayer::Move() {
    Matrix4x4 mat = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, Vector3{0.0f, cameraRotate.y, 0.0f}, playerTransform.translate);

    if (m_inputDirection.x != 0.0f)
    {
        if (Sign(moveVelocity.x) != m_inputDirection.x)
        {
            moveVelocity.x += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * m_inputDirection.x * 2.0f;
            moveVelocity.x = clamp(moveVelocity.x, -m_moveSpeed, m_moveSpeed);
        }
        else
        {
            moveVelocity.x += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * m_inputDirection.x;
            moveVelocity.x = clamp(moveVelocity.x, -m_moveSpeed, m_moveSpeed);
        }
    }
    else if(moveVelocity.x > 0.0f)
    { 
        moveVelocity.x -= m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * 3.0f;
        if (moveVelocity.x < 0.0f)
        {
            moveVelocity.x = 0.0f;
        }
    }
    else if (moveVelocity.x < 0.0f)
    {
        moveVelocity.x += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * 3.0f;
        if (moveVelocity.x > 0.0f)
        {
            moveVelocity.x = 0.0f;
        }
    }

    if (m_inputDirection.y != 0.0f)
    {
        if (Sign(moveVelocity.x) != m_inputDirection.x)
        {
            moveVelocity.z += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * m_inputDirection.y * 2.0f;
            moveVelocity.z = clamp(moveVelocity.z, -m_moveSpeed, m_moveSpeed);
        }
        else
        {
            moveVelocity.z += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * m_inputDirection.y;
            moveVelocity.z = clamp(moveVelocity.z, -m_moveSpeed, m_moveSpeed);
        }
    }
    else if (moveVelocity.z > 0.0f)
    {
        moveVelocity.z -= m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * 3.0f;
        if (moveVelocity.z < 0.0f)
        {
            moveVelocity.z = 0.0f;
        }
    }
    else if (moveVelocity.z < 0.0f)
    {
        moveVelocity.z += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime * 3.0f;
        if (moveVelocity.z > 0.0f)
        {
            moveVelocity.z = 0.0f;
        }
    }



    Vector3 normalDir = TransformNormal(moveVelocity, mat);
    m_velocity = normalDir;
}

// 重力の適用
void ActionPlayer::ApplyGravity() {
    if (!m_isGround)
    {
        m_velocity.y -= m_gravity * GameTime::GetInstance()->GetDeltaTime();
    }
    if (playerTransform.translate.y < 1.0f)
    {
        playerTransform.translate.y = 1.0f;
        m_isGround = true;
        m_velocity.y = 0.0f;
    }
}

// ジャンプ処理
void ActionPlayer::Jump() {
    if (m_pInput->PushKey(DIK_SPACE) && m_isGround)
    {
        m_isGround = false;
        float jumpVelocity = sqrtf(2.0f * m_gravity * GameTime::GetInstance()->GetDeltaTime() * m_jumpForce);
        m_velocity.y = jumpVelocity;
    }
}

// しゃがみ切り替え
void ActionPlayer::Crouch(bool IsPressed) {

}

// 壁との接触判定
bool ActionPlayer::CheckWall() {
	return false;
}

// 壁走り開始処理
void ActionPlayer::StartWallRun() {

}

// 壁走り終了処理
void ActionPlayer::StopWallRun() {

}

// 壁ジャンプ処理
void ActionPlayer::WallJump() {

}

// 地面判定
void ActionPlayer::CheckGround() {

}

// 各種状態の更新
void ActionPlayer::UpdateStates() {

}

// マウスによる視点操作
void ActionPlayer::HandleMouseLock() {
    cameraRotate = m_pCamera->GetRotate();

    cameraRotate.y += m_pInput->GetMouseVel2().x / 100.0f;
    cameraRotate.x += m_pInput->GetMouseVel2().y / 100.0f;

    cameraRotate.x = clamp(cameraRotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

}

bool showImGui = false;
void ActionPlayer::Debug() {
    if (m_pInput->TriggerKey(DIK_F6)) showImGui = !showImGui;
    if (!showImGui) return;
    ImGui::Begin("PlayerStetas");
    ImGui::DragFloat3("Translate", &playerTransform.translate.x, 0.1f);
    ImGui::DragFloat3("Velocity", &m_velocity.x, 0.1f);
    ImGui::DragFloat2("InputDirection", &m_inputDirection.x, 0.0f);
    ImGui::DragFloat("速度", &m_walkSpeed, 0.1f);
    ImGui::DragFloat("ジャンプ量", &m_jumpForce, 0.1f);
    ImGui::DragFloat("重力加速度", &m_gravity, 0.1f);
    ImGui::DragFloat2("平行移動用Velocity", &moveVelocity.x, 0.1f);

    ImGui::End();
}

