#include "Player.h"
#include "kMath.h"
#include "ImGuiManager.h"
#include "CollisionManager.h"
#include "WinApp.h"
#include "DirectXBase.h"

Player::~Player()
{
	delete playerModel_;
	CollisionManager::GetInstance()->DeleteCollisionTarget("player");
}

void Player::Initialize(Camera* camera, Input* input, const Transform startPoint, const bool DebugMode)
{
	debugMode_ = DebugMode;
	this->camera = camera;
	fovY_ = this->camera->GetfovY();
	//this->camera->SetTranslate({ 0.0f, 1.7f, 0.15f });
	this->input = input;
	parent_ = !DebugMode;

	playerTransform_ = startPoint;

	moveVelocity_ = { 0.0f, 0.0f, 0.0f };

	playerModel_ = new Object3d();
	playerModel_->Initialize();
	playerModel_->SetModel("Resources/Model/gltf/char", "idle.gltf", true, true);
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk.gltf", "walk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "sneak.gltf", "sneak");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "dash.gltf", "dash");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "jump.gltf", "jump");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "crouch.gltf", "crouch");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk_back.gltf", "backwalk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "fall.gltf", "fall");
	playerModel_->ToggleStartAnimation();

	playerAABB_ = playerModel_->GetAABB();

	CollisionManager::GetInstance()->AddCollisionTarget(playerAABB_, "player");
}

void Player::Update() {
	cameraTransform = camera->GetTransform();
	playerTransform_ = playerModel_->GetTransform();
	playerAABB_ = playerModel_->GetAABB();
	if (parent_)
	{
		// プレイヤーの回転からcameraOffsetを計算してparent
		Vector3 position = playerModel_->GetJointPosition("Head");

		Vector3	camOffset = cameraOffset_;

		// 一時的にrotate.xを格納しておく(上下の計算をしないため)
		float rotx = playerTransform_.rotate.x;
		playerTransform_.rotate.x = 0.0f;

		Matrix4x4 matrix = MakeAffineMatrix(playerTransform_);
		camOffset = TransformNormal(camOffset, matrix);
		position += camOffset;
		playerTransform_.rotate.x = rotx;
		Matrix4x4 world = { 0.0f };
		world.m[3][0] = position.x;
		world.m[3][1] = position.y;
		world.m[3][2] = position.z;
		camera->SetTranslateParent(world);
	}
	else
	{
		camera->DeleteTranslateParent();
	}
	Rotation();
	Move();

	playerModel_->SetAnimationSpeed(1.0f);
	playerModel_->SetTransform(playerTransform_);
	playerModel_->SetAnimationSpeed(1.0f);
	playerModel_->SetTransform(playerTransform_);
	playerModel_->Update();
	playerModel_->Update();

	if (debugMode_)
	{
		DebugUpdate();
	}
}

void Player::Draw() {
	/*if (moveType_ != PlayerMoveType::Jump)
	{
		playerModel_->Draw();
	}*/
}

const bool Player::IsClear() const
{
	float playerDist = Distance(playerTransform_.translate, { 0.0f, playerTransform_.translate.y, 0.0f });

	if (playerDist > clearDistance_)
	{
		return true;
	}
	return false;
}

void Player::Rotation() {
	Vector3 mouseVelocity = input->GetMouseVel3();

	mouseVelocity.x *= cameraSpeed.x / 100.0f;
	mouseVelocity.y *= cameraSpeed.y / 100.0f;


	cameraTransform.rotate.x += mouseVelocity.y;
	cameraTransform.rotate.y += mouseVelocity.x;

	cameraTransform.rotate.x = std::clamp(cameraTransform.rotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));

	camera->SetTransform(cameraTransform);

}

void Player::Move()
{
	if (input->PushKey(DIK_W))
	{
		playerTransform_.translate += camera->GetDirection() * translateSpeed_ * DirectXBase::GetInstance()->GetDeltaTime();
	}
}

void Player::Sneak()
{
}

void Player::DebugUpdate()
{
	Transform transform = camera->GetTransform();
	ImGui::Begin("Animation");
	ImGui::SetWindowPos(ImVec2{ 0.0f, 18.0f * 3.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) - 18.0f * 3.0f });
	ImGui::Checkbox("カメラ移動", &cameraMove_);
	ImGui::Checkbox("カメラ追従", &parent_);
	ImGui::DragFloat3("カメラオフセット", &cameraOffset_.x, 0.1f);
	//ImGui::DragFloat3("移動量", &speed_.x);
	ImGui::DragFloat3("MoveVelocity", &moveVelocity_.x, 0.1f);
	ImGui::DragFloat3("Translate", &playerTransform_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotate", &playerTransform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &playerTransform_.scale.x, 0.1f);
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = currentTime - lastTime;
	lastTime = currentTime;
	float time = delta.count();
	ImGui::DragFloat("DeltaTime", &time, 0.1f);
	//ImGui::DragFloat("最大落下速度", &fallLimit_, 0.1f);
	//ImGui::DragFloat("ジャンプ量", &jumpAcceleration_, 0.1f);
	//ImGui::DragFloat("落下量", &fallAcceleration_, 0.1f);
	ImGui::DragFloat("視野角", &normalFovY_, 0.01f);
	ImGui::DragFloat("視野角の上昇値", &fovYBoost_, 0.01f);
	float dist = CollisionManager::GetInstance()->GetGroundDistance("player");
	ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "GroundDistance: %.1f", dist);
	Vector3 penetrationAmount = CollisionManager::GetInstance()->GetPenetration();
	ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "PenetrationAmount : X=%.1f Y=%.1f  Z=%.1f", penetrationAmount.x, penetrationAmount.y, penetrationAmount.z);

	ImGui::End();

	if (input->TriggerKey(DIK_R))
	{
		playerModel_->SetTranslate({ 0.0f, 1.0f, 0.0f });
	}
	if (cameraMove_)
	{
		float speed = 0.4f;
		// カメラのY軸回転角度のみを使用
		float cameraYRotation = transform.rotate.y;

		// 前後移動（水平面のみ）
		Vector3 forward = {
			sinf(cameraYRotation) * speed,
			0.0f,
			cosf(cameraYRotation) * speed
		};

		// 左右移動（水平面のみ）
		Vector3 right = {
			cosf(cameraYRotation) * speed,
			0.0f,
			-sinf(cameraYRotation) * speed
		};

		if (input->PushKey(DIK_W)) {
			transform.translate += forward;
		}
		if (input->PushKey(DIK_S)) {
			transform.translate -= forward;
		}
		if (input->PushKey(DIK_A)) {
			transform.translate -= right;
		}
		if (input->PushKey(DIK_D)) {
			transform.translate += right;
		}
		if (input->PushKey(DIK_SPACE)) {
			transform.translate.y += speed;
		}
		if (input->PushKey(DIK_LSHIFT)) {
			transform.translate.y -= speed;
		}
		if (input->PushKey(DIK_LEFT)) {
			transform.rotate.y -= 0.03f;
		}
		if (input->PushKey(DIK_RIGHT)) {
			transform.rotate.y += 0.03f;
		}
		if (input->PushKey(DIK_UP)) {
			transform.rotate.x -= 0.03f;
		}
		if (input->PushKey(DIK_DOWN)) {
			transform.rotate.x += 0.03f;
		}
		if (input->PushKey(DIK_Q)) {
			transform.rotate.z -= 0.01f;
		}
		if (input->PushKey(DIK_E)) {
			transform.rotate.z += 0.01f;
		}
		if (input->PushKey(DIK_F1))
		{
			speed -= 0.005f;
		}
		if (input->PushKey(DIK_F2))
		{
			speed += 0.005f;
		}
	}

	camera->SetTransform(transform);
}
