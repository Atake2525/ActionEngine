#include "ActionPlayer.h"
#include "JsonLoader.h"
#include "GameTime.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "CollisionManager.h"

#ifndef NDEBUG	
#include "Logger.h"
using namespace Logger;
#endif // !NDEBUG


using namespace std;

ActionPlayer::ActionPlayer()
    : m_velocity({ 0.0f, 0.0f, 0.0f })
    , m_accelTime(0.4f)
    , m_moveSpeed(0.0f)
    , m_walkSpeed(0.15f)
    , m_dashSpeed(0.3f)
    , m_crouchSpeed(0.08f)
    , m_jumpForce(3.0f)
    , m_gravity(0.8f)
    , m_inputDirection({ 0.0f, 0.0f })
    , m_isGround(false)
    , m_isDash(false)
    , m_isCrouch(false)
    , m_isWallRun(false)
    , m_isWallJump(false)
    , m_wallNormal({ 0.0f, 0.0f, 0.0f })
    , m_groundDistance(0.0f)
    , m_wallDistance(0.0f)
    , m_walkFov(1.0f)
    , m_dashFov(1.3f)
    , m_fovChangeTime(0.1f)
    , m_playerAABB({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f })
{
}

Vector3 cameraRotate;
Transform playerTransform;
Vector3 panetration = { 0.0f, 0.0f, 0.0f };

void ActionPlayer::Initialize(Camera* camera) {
    m_pPlayerModel = make_unique<Object3d>();
    m_pPlayerModel->Initialize();
    m_pPlayerModel->SetModel("Resources/Model/gltf/Player", "PlayerCollision.gltf");
    playerTransform = m_pPlayerModel->GetTransform();
    CollisionManager::GetInstance()->AddCollisionTarget(m_playerAABB, "player");
    m_pCamera = camera;

    m_pInput = Input::GetInstance();
}

bool firstUpdate = false;
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

    playerTransform.translate += m_velocity;
    ApplyCollision();

    /* if (CheckWall()) {
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

    m_pPlayerModel->SetTranslate({ playerTransform.translate.x, playerTransform.translate.y, playerTransform.translate.z + 0.05f });
    m_pPlayerModel->Update();

    if (!firstUpdate)
    {
        m_pCamera->SetTranslate({ 0.0f, m_pPlayerModel->GetAABB().max.y - 0.2f, 0.0f });
        firstUpdate = true;
    }

    UpdateEffects();

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
float velocityLerpTimer = 0.0f;
// 移動処理 (地上・空中)
void ActionPlayer::Move() {
    m_playerAABB = m_pPlayerModel->GetAABB();

    moveVelocity.x = m_inputDirection.x;
    moveVelocity.z = m_inputDirection.y;

    if (m_inputDirection.x != 0.0f && m_inputDirection.y != 0.0f)
    {
        Vector3 vel = m_velocity;
        float len = Length(moveVelocity);
        if (len == 0.0f)
        {
            len = 1.0f;
        }
        moveVelocity = moveVelocity / len;
    }

    Matrix4x4 mat = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, Vector3{ 0.0f, cameraRotate.y, 0.0f }, playerTransform.translate);
    Vector3 normalDir = TransformNormal(moveVelocity, mat);
    Vector3 clampSpeedDir = Normalize(normalDir);

    if (m_pInput->TriggerKey(DIK_T))
    {
        float t = 0;
    }

    // 速度を落とす
    if (m_velocity.x > 0.0f && m_inputDirection.x == 0.0f)
    {
        m_velocity.x -= m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime;
        if (m_velocity.x < 0.0f)
        {
            m_velocity.x = 0.0f;
        }
    }
    else if (m_velocity.x < 0.0f && m_inputDirection.x == 0.0f)
    {
        m_velocity.x += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime;
        if (m_velocity.x > 0.0f)
        {
            m_velocity.x = 0.0f;
        }
    }
    if (m_velocity.z > 0.0f && m_inputDirection.y == 0.0f)
    {
        m_velocity.z -= m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime;
        if (m_velocity.z < 0.0f)
        {
            m_velocity.z = 0.0f;
        }
    }
    else if (m_velocity.z < 0.0f && m_inputDirection.y == 0.0f)
    {
        m_velocity.z += m_moveSpeed * GameTime::GetInstance()->GetDeltaTime() / m_accelTime;
        if (m_velocity.z > 0.0f)
        {
            m_velocity.z = 0.0f;
        }
    }

    // 移動用Velocityに格納
    m_velocity.x += normalDir.x * GameTime::GetInstance()->GetDeltaTime();
    m_velocity.z += normalDir.z * GameTime::GetInstance()->GetDeltaTime();

    // Clamp
    clampSpeedDir.x *= Sign(clampSpeedDir.x);
    clampSpeedDir.z *= Sign(clampSpeedDir.z);
    Vector3 mapSpeed = { m_moveSpeed * clampSpeedDir.x, m_moveSpeed * clampSpeedDir.y, m_moveSpeed * clampSpeedDir.z };
    if (clampSpeedDir.x == 0.0f || clampSpeedDir.z == 0.0f)
    {
        m_velocity.x = clamp(m_velocity.x, -m_moveSpeed, m_moveSpeed);
        m_velocity.z = clamp(m_velocity.z, -m_moveSpeed, m_moveSpeed);
    }
    else
    {
        m_velocity.x = clamp(m_velocity.x, -mapSpeed.x, mapSpeed.x);
        m_velocity.z = clamp(m_velocity.z, -mapSpeed.z, mapSpeed.z);
    }

}

// 重力の適用
void ActionPlayer::ApplyGravity() {

    if (CollisionManager::GetInstance()->GetGroundDistance("player") > 0.2f && m_isGround)
    {
        m_isGround = false;
    }

    if (!m_isGround)
    {
        m_velocity.y -= m_gravity * GameTime::GetInstance()->GetDeltaTime();
        m_velocity.y = clamp(m_velocity.y, -2.4f, 10.0f);
    }

    if (panetration.y < 0.0f)
    {
        m_isGround = true;
        m_velocity.y = 0.0f;
    }

}

// ジャンプ処理
void ActionPlayer::Jump() {
    if (m_velocity.y < 0.0f && !m_isGround && CheckWall())
    {
        if (m_velocity.x > 0.0f || m_velocity.z > 0.0f)
        {
            cameraRotate.z = SwapRadian(35.0f);
        }
        else if (m_velocity.x < 0.0f || m_velocity.z < 0.0f)
        {
            cameraRotate.z = -SwapRadian(35.0f);
        }
        else
        {
            cameraRotate.z = 0.0f;
        }
    }
    else
    {
        cameraRotate.z = 0.0f;
    }


    if (m_pInput->PushKey(DIK_SPACE) && m_isGround)
    {
        m_isGround = false;
        float jumpVelocity = sqrtf(2.0f * m_gravity * GameTime::GetInstance()->GetDeltaTime() * m_jumpForce);
        m_velocity.y = jumpVelocity;
    }
}

// 当たり判定の適用
void ActionPlayer::ApplyCollision() {
    m_playerAABB += m_velocity;
    CollisionManager::GetInstance()->UpdateCollisionTarget(m_playerAABB, "player");
    CollisionManager::GetInstance()->Update("player");
    panetration = CollisionManager::GetInstance()->GetPenetration();
    playerTransform.translate -= panetration;
}

// しゃがみ切り替え
void ActionPlayer::Crouch() {

}

float wallCheckHeihgt = -2.0f; // 壁走りが可能な壁の高さを設定(欲しい高さ * -1.0f)
// 壁との接触判定
bool ActionPlayer::CheckWall() {
    if (CollisionManager::GetInstance()->GetGroundDistance("player") <= -2.0f)
    {
        return true;
    }
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

    cameraRotate.y += m_pInput->GetMouseVel2().x / 100.0f / 10.0f;
    cameraRotate.x += m_pInput->GetMouseVel2().y / 100.0f / 10.0f;

    cameraRotate.x = clamp(cameraRotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

}

bool changeFov = false;
float fovChangeTimer = 0.0f;
float currentFov = 0.0f;
float fovEnd = 0.0f;
// 演出系更新
void ActionPlayer::UpdateEffects() {
    if (!changeFov)
    {
        currentFov = m_pCamera->GetfovY();
        if (currentFov != m_dashFov && (moveVelocity.x * Sign(moveVelocity.x) == m_dashSpeed || moveVelocity.z * Sign(moveVelocity.z) == m_dashSpeed))
        {
            changeFov = true;
            fovEnd = m_dashFov;
        }
        else if (currentFov != m_walkFov && (moveVelocity.x * Sign(moveVelocity.x) <= m_walkSpeed && moveVelocity.z * Sign(moveVelocity.z) <= m_walkSpeed))
        {
            changeFov = true;
            fovEnd = m_walkFov;
        }
    }
    else
    {
        fovChangeTimer += GameTime::GetInstance()->GetDeltaTime() / m_fovChangeTime;
        fovChangeTimer = clamp(fovChangeTimer, 0.0f, 1.0f);
        float fov = Lerp(currentFov, fovEnd, fovChangeTimer);
        m_pCamera->SetFovY(fov);
        if (fovChangeTimer == 1.0f)
        {
            changeFov = false;
            fovChangeTimer = 0.0f;
        }
    }

}


bool showImGui = true;
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
    ImGui::DragFloat("LerpTimer", &velocityLerpTimer, 0.0f);
    auto groundDist = CollisionManager::GetInstance()->GetGroundDistance("player");
    ImGui::DragFloat("GroundDistance", &groundDist, 0.0f);
    auto panetration = CollisionManager::GetInstance()->GetPenetration();
    ImGui::DragFloat3("Penetration", &panetration.x, 0.0f);

    ImGui::End();
}

