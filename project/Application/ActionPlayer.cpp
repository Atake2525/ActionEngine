#include "ActionPlayer.h"
#include "JsonLoader.h"
#include "GameTime.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "CollisionManager.h"
#include "EasingUtility.h"

#ifndef NDEBUG	
#include "Logger.h"
using namespace Logger;
#endif // !NDEBUG


using namespace std;

ActionPlayer::~ActionPlayer() {
    CollisionManager::GetInstance()->DeleteCollisionTarget("player");
}

ActionPlayer::ActionPlayer()
    : m_velocity(Vector3::Zero)
    , m_accelTime(0.4f)
    , m_moveSpeed(0.0f)
    , m_walkSpeed(11.0f)
    , m_dashSpeed(18.0f)
    , m_crouchSpeed(8.0f)
    , m_jumpForce(3.0f)
    , m_wallJumpForce(15.0f)
    , m_gravity(0.8f)
    , m_fallLimit(-2.4f)
    , m_inputDirection({ 0.0f, 0.0f })
    , m_isGround(false)
    , m_isDash(false)
    , m_isCrouch(false)
    , m_isWallRun(false)
    , m_isWallJump(false)
    , m_wallNormal(Vector3::Zero)
    , m_groundDistance(0.0f)
    , m_wallDistance(0.0f)
    , m_walkFov(1.0f)
    , m_dashFov(1.3f)
    , m_fovChangeTime(0.1f)
    , m_playerAABB(Vector3::Zero, Vector3::Zero)
{
}

Vector3 cameraRotate;
Transform playerTransform;
Vector3 panetration = Vector3::Zero;

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

Vector3 moveVelocity = Vector3::Zero;
Vector3 vel = Vector3::Zero;
Vector3 jumpVelocity = Vector3::Zero;

void ActionPlayer::Update() {
    m_velocity = Vector3::Zero;

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

    m_velocity = moveVelocity + jumpVelocity;
    float maxSpeed = m_moveSpeed * GameTime::GetInstance()->GetDeltaTime();
    maxSpeed *= Sign(maxSpeed);

    m_velocity.x = clamp(m_velocity.x, -maxSpeed, maxSpeed);
    m_velocity.z = clamp(m_velocity.z, -maxSpeed, maxSpeed);
    playerTransform.translate += m_velocity;
    ApplyCollision();

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


float velocityLerpTimer = 0.0f;
Vector3 moveVel = Vector3::Zero;
float jumpMoveSign = 0.0f;
// 移動処理 (地上・空中)
void ActionPlayer::Move() {
    m_playerAABB = m_pPlayerModel->GetAABB();

    // 左右移動のみ壁ジャンプ中は操作を受け付けないようにする
    if (!m_isWallJump)
    {
        if (m_inputDirection.x != 0.0f)
        {
            float sign = Sign(m_inputDirection.x);
            float moveSign = Sign(moveVel.x);
            if (moveSign == sign)
            {
                moveVel.x += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 2.0f * sign;
            }
            else
            {
                moveVel.x += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 4.0f * sign;
            }
        }
        else
        {
            if (moveVel.x > 0.0f)
            {
                moveVel.x -= m_accelTime * GameTime::GetInstance()->GetDeltaTime();
                if (moveVel.x < 0.0f)
                {
                    moveVel.x = 0.0f;
                }
            }
            else if (moveVel.x < 0.0f)
            {
                moveVel.x += m_accelTime * GameTime::GetInstance()->GetDeltaTime();
                if (moveVel.x > 0.0f)
                {
                    moveVel.x = 0.0f;
                }
            }
        }
    }
    else
    {
        // JumpVelocityの値をいじる
        // InputDirectionとCameraの向きから計算
        if (Sign(m_inputDirection.x) * Sign(m_pCamera->GetDirection().z) != Sign(jumpVelocity.x))
        {
            float sign = Sign(m_inputDirection.x) * Sign(m_pCamera->GetDirection().z);
            float moveSign = Sign(moveVel.x);
            if (moveSign == sign)
            {
                jumpVelocity.x += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 2.0f * sign;
            }
            else
            {
                jumpVelocity.x += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 4.0f * sign;
                if (jumpVelocity.x < 0.0f)
                {

                }
            }
            /*if (Sign(jumpVelocity.x) != jumpMoveSign)
            {
                jumpVelocity.x = 0.0f;
                m_isWallJump = false;
            }*/
        }
    }
    if (m_inputDirection.y != 0.0f)
    {
        float sign = Sign(m_inputDirection.y);
        float moveSign = Sign(moveVel.z);
        if (moveSign == sign)
        {
            moveVel.z += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 2.0f * sign;
        }
        else
        {
            moveVel.z += m_accelTime * GameTime::GetInstance()->GetDeltaTime() * 4.0f * sign;
        }
    }
    else
    {
        if (moveVel.z > 0.0f)
        {
            moveVel.z -= m_accelTime * GameTime::GetInstance()->GetDeltaTime();
            if (moveVel.z < 0.0f)
            {
                moveVel.z = 0.0f;
            }
        }
        else if (moveVel.z < 0.0f)
        {
            moveVel.z += m_accelTime * GameTime::GetInstance()->GetDeltaTime();
            if (moveVel.z > 0.0f)
            {
                moveVel.z = 0.0f;
            }
        }
    }

    float maxSpeed = m_moveSpeed * GameTime::GetInstance()->GetDeltaTime();
    maxSpeed *= Sign(maxSpeed);

    if (m_inputDirection.x != 0.0f && m_inputDirection.y != 0.0f)
    {
        Vector3 DirVel = { m_inputDirection.x, 0.0f, m_inputDirection.y };
        float len = Length(DirVel);
        if (len == 0.0f)
        {
            len = 1.0f;
        }
        maxSpeed = maxSpeed / len;
    }

    moveVel.x = clamp(moveVel.x, -maxSpeed, maxSpeed);
    moveVel.z = clamp(moveVel.z, -maxSpeed, maxSpeed);

    Matrix4x4 mat = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, Vector3{ 0.0f, cameraRotate.y, 0.0f }, playerTransform.translate);
    Vector3 normalDir = TransformNormal(moveVel, mat);

    moveVelocity.x = normalDir.x;
    moveVelocity.z = normalDir.z;

    // 入力方向を正規化
    //Vector3 inputDir = { m_inputDirection.x, 0.0f, m_inputDirection.y };
    //if (inputDir.x != 0.0f && inputDir.z != 0.0f)
    //{
    //    float len = Length(inputDir);
    //    inputDir = inputDir / len;
    //}

    //// 視点の方向に応じて移動量を変えたいのでTransformNormalを行う
    //Matrix4x4 rotMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, Vector3{ 0.0, cameraRotate.y, 0.0f }, playerTransform.translate);
    //Vector3 moveDir = TransformNormal(inputDir, rotMatrix);

    //// 入力方向を使って最高速度を計算
    ////Vector3 SignDir = Sign(moveDir);
    //Vector3 maxSpeed = moveDir * m_moveSpeed * GameTime::GetInstance()->GetUnFixedDeltaTime();
    //maxSpeed *= Sign(maxSpeed);

    //moveVel += moveDir * (1.0f - exp(-m_accelTime * GameTime::GetInstance()->GetDeltaTime()));

    //if (m_inputDirection.x != 0.0f)
    //{
    //    float t = 0.0f;
    //}

    //moveVel.x = clamp(moveVel.x, -maxSpeed.x, maxSpeed.x);
    //moveVel.z = clamp(moveVel.z, -maxSpeed.z, maxSpeed.z);


    //moveVelocity.x = moveVel.x;
    //moveVelocity.z = moveVel.z;

}

// 重力の適用
void ActionPlayer::ApplyGravity() {

    if (CollisionManager::GetInstance()->GetGroundDistance("player") > 0.2f && m_isGround)
    {
        m_isGround = false;
    }

    if (!m_isGround)
    {
        if (!m_isWallRun)
        {
            moveVelocity.y -= m_gravity * GameTime::GetInstance()->GetDeltaTime();
            moveVelocity.y = clamp(moveVelocity.y, m_fallLimit, 10.0f);
        }
        else
        {
            moveVelocity.y = -m_gravity * 1.2f * GameTime::GetInstance()->GetDeltaTime();
        }
    }

    if (panetration.y < 0.0f && moveVelocity.y < 0.0f)
    {
        m_isGround = true;
        m_isWallJump = false;
        moveVelocity.y = 0.0f;
        moveVelocity.x = jumpVelocity.x;
        moveVelocity.z = jumpVelocity.z;
        jumpVelocity = Vector3::Zero;
    }

}

// ジャンプ処理
void ActionPlayer::Jump() {
    if (moveVelocity.y < 0.0f && !m_isGround && CheckWall())
    {
        m_isWallRun = true;
    }
    else
    {
        m_isWallRun = false;
        cameraRotate.z = 0.0f;
    }

    // ジャンプ処理
    if (m_pInput->PushKey(DIK_SPACE))
    {
        // 通常のジャンプと壁ジャンプを分ける
        if (!m_isWallRun && m_isGround)
        {
            m_isGround = false;
            float jumpVel = sqrtf(2.0f * m_gravity * GameTime::GetInstance()->GetDeltaTime() * m_jumpForce);
            moveVelocity.y = jumpVel;
        }
        else if (m_isWallRun && !m_isGround)
        {
            m_isWallJump = true;
            m_isWallRun = false;
            moveVel.x = 0.0f;
            m_isGround = false;
            float jumpVel = sqrtf(2.0f * m_gravity * GameTime::GetInstance()->GetDeltaTime() * m_jumpForce);
            moveVelocity.y = jumpVel;
            float wallJumpVel = sqrtf(2.0f * m_accelTime * GameTime::GetInstance()->GetDeltaTime() * m_wallJumpForce);
            if (panetration.x != 0.0f)
            {
                jumpVelocity.x = -(wallJumpVel * Sign(panetration.x));
            }
            else if (panetration.z != 0.0f)
            {
                jumpVelocity.z = -(wallJumpVel * Sign(panetration.z));
            }
            jumpMoveSign = Sign(jumpVelocity.x);
        }
    }
}

// 当たり判定の適用
void ActionPlayer::ApplyCollision() {
    m_playerAABB += m_velocity;
    CollisionManager::GetInstance()->UpdateCollisionTarget(m_playerAABB, "player");
    CollisionManager::GetInstance()->Update("player");
    panetration = CollisionManager::GetInstance()->GetPenetration();
    if (panetration.x != 0.0f)
    {
        moveVel.x = 0.0f;
    }
    else if (panetration.z != 0.0f) {
        moveVel.z = 0.0f;
    }
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

    cameraRotate.y += m_pInput->GetMouseVel2().x / 100.0f / 5.0f;
    cameraRotate.x += m_pInput->GetMouseVel2().y / 100.0f / 5.0f;

    cameraRotate.x = clamp(cameraRotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

}

bool changeFov = false;
float fovChangeTimer = 0.0f;
float currentFov = 0.0f;
float fovEnd = 0.0f;

bool fallEffect = false;
float cameraYOffset = 0.0f;
float fallEffectTimer = 0.0f;
bool playFallEffect = false;

bool cameraRotateZEffect = false;
float cameraRotateZTimer = 0.0f;
float startRotateZ = 0.0f;
float endRotateZ = 0.0f;

// 演出系更新
void ActionPlayer::UpdateEffects() {
    // FOV変更
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

    // 落下エフェクト
    if (!m_isGround && moveVelocity.y < -0.5f)
    {
        fallEffect = true;
    }
    if (fallEffect && m_isGround)
    {
        cameraYOffset = m_pCamera->GetTranslate().y;
        m_pCamera->SetTranslate({ 0.0f, cameraYOffset - 1.0f, 0.0f });
        playFallEffect = true;
        fallEffect = false;
    }
    if (playFallEffect)
    {
        fallEffectTimer += GameTime::GetInstance()->GetDeltaTime() / 0.4f;
        fallEffectTimer = clamp(fallEffectTimer, 0.0f, 1.0f);
        float yOffset = EaseInOut(fallEffectTimer, cameraYOffset - 1.0f, cameraYOffset);
        m_pCamera->SetTranslate({ 0.0f, yOffset, 0.0f });
        if (fallEffectTimer == 1.0f)
        {
            playFallEffect = false;
            cameraYOffset = 0.0f;
            fallEffectTimer = 0.0f;
        }
    }

    //// 壁走り時のカメラ傾き
    //if (m_isWallRun && cameraRotate.z == 0.0f)
    //{
    //    cameraRotateZEffect = true;
    //}
    //if (moveVelocity.y < 0.0f && !m_isGround && CheckWall())
    //{
    //    Vector3 cameraDirection = m_pCamera->GetDirection();

    //    if (panetration.x != 0.0f)
    //    {
    //        startRotateZ = 0.0f;
    //        endRotateZ = SwapRadian(35.0f) * Sign(cameraDirection.z) * Sign(panetration.x);
    //    }
    //    else if (panetration.z != 0.0f)
    //    {
    //        startRotateZ = 0.0f;
    //        endRotateZ = -SwapRadian(35.0f) * Sign(cameraDirection.x) * Sign(panetration.z);
    //    }
    //}
    //if (cameraRotate.z != 0.0f && !m_isWallRun)
    //{
    //    startRotateZ = cameraRotate.z;
    //    endRotateZ = 0.0f;
    //    cameraRotateZEffect = true;
    //}
    //if (cameraRotateZEffect)
    //{
    //    cameraRotateZTimer += GameTime::GetInstance()->GetDeltaTime() / 0.2f;
    //    cameraRotateZTimer = clamp(cameraRotateZTimer, 0.0f, 1.0f);
    //    cameraRotate.z = Lerp(startRotateZ, endRotateZ, cameraRotateZTimer);
    //    if (cameraRotateZTimer > 1.0f)
    //    {
    //        cameraRotateZTimer = 0.0f;
    //        cameraRotateZEffect = false;
    //    }
    //}
}


bool showImGui = true;
void ActionPlayer::Debug() {

    //ImGui::ShowDemoWindow();

    if (m_pInput->TriggerKey(DIK_F6)) showImGui = !showImGui;
    if (!showImGui) return;
    ImGui::Begin("Player");
    ImGui::SetWindowPos(ImVec2{ 0.0f, 18.0f * 3.0f });
    ImGui::SetWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) - 18.0f * 3.0f });
    /*if (ImGui::BeginMenu("表示選択"))
    {
        ImGui::DragFloat3("Velocity", &m_velocity.x, 0.1f);
        ImGui::EndMenu();
    }
    ImGui::DragFloat3("Translate", &playerTransform.translate.x, 0.1f);
    if (ImGui::TreeNode("ステータス"))
    {

        ImGui::TreePop();
    }
   */
    ImGui::DragFloat3("Translate", &playerTransform.translate.x, 0.1f);
    ImGui::DragFloat3("Velocity", &m_velocity.x, 0.1f);
    ImGui::DragFloat3("MoveVelocity", &moveVelocity.x, 0.1f);
    ImGui::DragFloat3("MoveVel", &moveVel.x, 0.1f);
    ImGui::DragFloat3("JumpVelocity", &jumpVelocity.x, 0.1f);
    ImGui::DragFloat2("InputDirection", &m_inputDirection.x, 0.0f);
    ImGui::DragFloat("速度", &m_walkSpeed, 0.1f);
    ImGui::DragFloat("ジャンプ量", &m_jumpForce, 0.1f);
    ImGui::DragFloat("壁ジャンプ力", &m_wallJumpForce, 0.1f);
    ImGui::DragFloat("重力加速度", &m_gravity, 0.1f);
    ImGui::DragFloat2("平行移動用Velocity", &moveVelocity.x, 0.1f);
    ImGui::DragFloat("LerpTimer", &velocityLerpTimer, 0.0f);
    auto groundDist = CollisionManager::GetInstance()->GetGroundDistance("player");
    ImGui::DragFloat("GroundDistance", &groundDist, 0.0f);
    auto panetration = CollisionManager::GetInstance()->GetPenetration();
    ImGui::DragFloat3("Penetration", &panetration.x, 0.0f);
    ImGui::DragFloat("Walk", &m_walkSpeed, 0.1f);
    ImGui::DragFloat("Dash", &m_dashSpeed, 0.1f);
    ImGui::DragFloat("AccelTime", &m_accelTime, 0.1f);
    ImGui::DragFloat("Crouch", &m_crouchSpeed, 0.1f);

    ImGui::End();
}

