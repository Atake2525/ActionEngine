#include "Player.h"
#include "kMath.h"
#include "ImGuiManager.h"

Player::~Player()
{
    delete playerModel_;
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
    playerModel_->SetModel("Resources/Model/gltf/char", "noHeadIdle.gltf", true, true);
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk.gltf", "walk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "sneak.gltf", "sneak");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "dash.gltf", "dash");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "jump.gltf", "jump");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "crouch.gltf", "crouch");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk_back.gltf", "backwalk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "fall.gltf", "fall");
    playerModel_->SetTransform(playerTransform_);
    playerModel_->ToggleStartAnimation();
}

void Player::Update() {
	cameraTransform = camera->GetTransform();
	playerTransform_ = playerModel_->GetTransform();
	if (parent_)
	{
		// プレイヤーの回転からcameraOffsetを計算してparent
		Vector3 position = playerModel_->GetJointPosition("Head");

		Vector3	camOffset = cameraOffset_;

		if (moveType_ == PlayerMoveType::Dash)
		{
			camOffset.z += 0.04f;
		}
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

	if (input->TriggerKey(DIK_G))
	{
		playerModel_->CreateCapsule();
	}

	playerModel_->SetAnimationSpeed(1.0f);
	playerModel_->SetTransform(playerTransform_);
    playerModel_->Update();

	if (debugMode_)
	{
		DebugUpdate();
	}
}

void Player::Draw() {
    playerModel_->Draw();
}

void Player::Rotation() {
	Vector3 mouseVelocity = input->GetMouseVel3();

	mouseVelocity.x *= cameraSpeed.x / 100.0f;
	mouseVelocity.y *= cameraSpeed.y / 100.0f;


	cameraTransform.rotate.x += mouseVelocity.y;
	cameraTransform.rotate.y += mouseVelocity.x;

	camera->SetTransform(cameraTransform);

}

void Player::Move() 
{
	// 無操作状態ならば何もしないので毎フレームIdle状態にする
	moveType_ = PlayerMoveType::Idle;

	// ジャンプ状態の解除(仮)
	if (playerTransform_.translate.y < 0.0f)
	{
		playerTransform_.translate.y = 0.1f;
		jump_ = false;
		speed_.y = 0.0f;
	}

	// 一番最初にジャンプ状態の有無を調べる(ジャンプ中か否かで移動系処理が変わるため)
	if (input->PushKey(DIK_SPACE))
	{
		if (!jump_)
		{
			speed_.y = jumpAcceleration_;
		}
		jump_ = true;
	}
	if (jump_)
	{
		moveType_ = PlayerMoveType::Jump;
		speed_.y -= fallAcceleration_;
	}
	speed_.y = std::clamp(speed_.y, fallLimit_, jumpAcceleration_);
	
	// 移動の最高速度をここに格納する
	float maxSpeed_ = walkSpeed_ * speedLimit_;

	// ダッシュとスニークの判定
	// ダッシュよりもスニークを優先して判定させる
	if (input->PushKey(DIK_LCONTROL))
	{
		// プレイヤーの状態をしゃがみにする しゃがみ移動の処理は平行移動処理の後に書く
		// 最高移動速度をsneakSpeedにする
		moveType_ = PlayerMoveType::Crouch;
		maxSpeed_ = sneakSpeed_ * speedLimit_;
	}
	else if (input->PushKey(DIK_LSHIFT)) {
		// プレイヤーの状態をダッシュにするのは移動している時なので変更しない
		// 最高移動速度をdashSpeedにする
		maxSpeed_ = dashSpeed_ * speedLimit_;
	}

	if (jump_)
	{
		moveType_ = PlayerMoveType::Jump;
	}
	// accelerationを計算する
	float acceleration = 0.0f;
	if (moveType_ != PlayerMoveType::Jump)
	{
		acceleration = maxSpeed_ * translateAcceleration_;
	}
	else
	{
		acceleration = maxSpeed_ * flyAcceleration_;
	}

	// 平行移動(前後左右)
	if (input->PushKey(DIK_W))
	{
		moveType_ = PlayerMoveType::Walk;
		speed_.z += acceleration;
	}
	else if (speed_.z > 0.0f)
	{
		speed_.z -= acceleration * 1.5f;
		speed_.z = std::clamp(speed_.z, 0.0f, maxSpeed_);
	}

	if (input->PushKey(DIK_S))
	{
		moveType_ = PlayerMoveType::Backwalk;
		speed_.z -= acceleration;
	}
	else if (speed_.z < 0.0f)
	{
		speed_.z += acceleration * 1.5f;
		speed_.z = std::clamp(speed_.z, -maxSpeed_, 0.0f);
	}

	if (input->PushKey(DIK_D))
	{
		moveType_ = PlayerMoveType::Walk;
		speed_.x += acceleration;
	}
	else if (speed_.x > 0.0f)
	{
		speed_.x -= acceleration * 1.5f;
		speed_.x = std::clamp(speed_.x, 0.0f, maxSpeed_);
	}

	if (input->PushKey(DIK_A))
	{
		moveType_ = PlayerMoveType::Walk;
		speed_.x -= acceleration;
	}
	else if (speed_.x < 0.0f)
	{
		speed_.x += acceleration * 1.5f;
		speed_.x = std::clamp(speed_.x, -maxSpeed_, 0.0f);
	}

	speed_.x = std::clamp(speed_.x, -maxSpeed_, maxSpeed_);
	speed_.z = std::clamp(speed_.z, -maxSpeed_, maxSpeed_);

	moveVelocity_ = speed_;
	// 斜め移動の場合はXとZを正規化する
	if (speed_.x != 0.0f && speed_.z != 0.0f)
	{
		// ジャンプ中の可能性を考慮してNormalizeのときはYの値を別の場所に格納しておく
		float y = speed_.y;
		speed_.y = 0.0f;
		// Normalizeして移動方向を正規化しているのでmaxSpeed_を掛けて速度を期待する数値へもどす
		moveVelocity_ = Normalize(speed_) * maxSpeed_;
		speed_.y = y;
		moveVelocity_.y = speed_.y;
	}

	// しゃがみとダッシュは移動しているかどうかでも状態(アニメーション)が変わるため移動処理の後に調べる
	if (maxSpeed_ == sneakSpeed_ * speedLimit_ && (speed_.x != 0.0f || speed_.z != 0.0f)) // しゃがみの最高速度なら
	{
		// プレイヤーの状態をしゃがみにする しゃがみ移動の処理は平行移動処理の後に書く
		// 最高移動速度をsneakSpeedにする
		moveType_ = PlayerMoveType::Sneak;
	}
	else if (maxSpeed_ == dashSpeed_ * speedLimit_ && (speed_.x != 0.0f || speed_.z != 0.0f)) { // ダッシュの最高速度なら
		// プレイヤーの状態をダッシュにする
		// 最高移動速度をdashSpeedにする
		moveType_ = PlayerMoveType::Dash;
	}

	if (jump_)
	{
		moveType_ = PlayerMoveType::Jump;
	}

	// カメラの方向を調べて移動方向を決める
	cameraTransform = camera->GetTransform();
	// 上下に移動はしないためrotate.xは0.0fにする
	cameraTransform.rotate.x = 0.0f;
	// TransformNormalのために3次元アフィン変換行列を作成
	Matrix4x4 matrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	// 回転行列を参照して移動ベクトルを正規化する
	moveVelocity_ = TransformNormal(moveVelocity_, matrix);

	// プレイヤーの移動量を今のプレイヤーの位置に加算する
	playerTransform_.translate += moveVelocity_;
	// プレイヤーの回転をカメラの正面を向くように変える
	playerTransform_.rotate = cameraTransform.rotate;

	// 速度が歩行状態よりも早ければ速度が上がっている感を出すためにFovを上げる(ジャンプ中はFovが増えないようにする)
	if (((speed_.x == dashSpeed_ * speedLimit_ || speed_.z == dashSpeed_ * speedLimit_) || (speed_.x == -(dashSpeed_ * speedLimit_) || speed_.z == -(dashSpeed_ * speedLimit_))) && !jump_)
	{
		fovTime_ = 0.0f;
		afterFovY_ = 0.65f;
	}
	else
	{
		fovTime_ = 0.0f;
		afterFovY_ = 0.45f;
	}
	// Fovの保管計算(一瞬でFovの数値が変わらないようにする)
	fovTime_ += (1.0f / 60.0f) / 0.2f;
	fovTime_ = std::clamp(fovTime_, 0.0f, 1.0f);
	fovY_ = Lerp(camera->GetfovY(), afterFovY_, fovTime_);

	// 計算結果をカメラにセット
	camera->SetFovY(fovY_);

	// プレイヤーの入力に応じてアニメーションを変える
	if (moveType_ != moveTypePre_)
	{
		switch (moveType_)
		{
		case PlayerMoveType::Idle:
			playerModel_->ChangePlayAnimation();
			break;
		case PlayerMoveType::Crouch:
			playerModel_->SetChangeAnimationSpeed(0.14f);
			playerModel_->ChangePlayAnimation("crouch");
			break;
		case PlayerMoveType::Walk:
			playerModel_->SetChangeAnimationSpeed();
			playerModel_->ChangePlayAnimation("walk");
			break;
		case PlayerMoveType::Backwalk:
			playerModel_->SetChangeAnimationSpeed();
			playerModel_->SetAnimationSpeed(20.0f);
			playerModel_->ChangePlayAnimation("backwalk");
			break;
		case PlayerMoveType::Sneak:
			playerModel_->SetChangeAnimationSpeed(0.18f);
			playerModel_->ChangePlayAnimation("sneak");
			break;
		case PlayerMoveType::Dash:
			playerModel_->SetChangeAnimationSpeed(0.2f);
			playerModel_->ChangePlayAnimation("dash");
			break;
		case PlayerMoveType::Jump:
			playerModel_->SetChangeAnimationSpeed(0.1f);
			playerModel_->ChangePlayAnimation("fall");
			break;
		}
	}
	moveTypePre_ = moveType_;
}

void Player::Sneak()
{
}

void Player::DebugUpdate()
{
	Transform transform = camera->GetTransform();
	ImGui::Begin("Animation");
	if (ImGui::Button("Idle"))
	{
		moveType_ = PlayerMoveType::Idle;
	}
	if (ImGui::Button("Walk"))
	{
		moveType_ = PlayerMoveType::Walk;
	}
	if (ImGui::Button("Sneak"))
	{
		moveType_ = PlayerMoveType::Sneak;
	}
	if (ImGui::Button("Dash"))
	{
		moveType_ = PlayerMoveType::Dash;
	}
	ImGui::Checkbox("カメラ移動", &cameraMove_);
	ImGui::Checkbox("カメラ追従", &parent_);
	ImGui::DragFloat3("カメラオフセット", &cameraOffset_.x, 0.1f);
	ImGui::DragFloat3("移動量", &speed_.x);
	ImGui::DragFloat3("MoveVelocity", &moveVelocity_.x, 0.1f);
	ImGui::DragFloat3("Translate", &playerTransform_.translate.x, 0.1f);
	ImGui::DragFloat3("Rotate", &playerTransform_.rotate.x, 0.1f);
	ImGui::DragFloat3("Scale", &playerTransform_.scale.x, 0.1f);
	ImGui::DragFloat("最大落下速度", &fallLimit_, 0.1f);
	ImGui::DragFloat("ジャンプ量", &jumpAcceleration_, 0.1f);
	ImGui::DragFloat("落下量", &fallAcceleration_, 0.1f);


	ImGui::End();

	if (cameraMove_)
	{
		float speed = 0.4f;
		Vector3 velocity(0.0f, 0.0f, speed);
		velocity = TransformNormal(velocity, camera->GetWorldMatrix());
		if (input->PushKey(DIK_W)) {
			transform.translate += velocity;
		}
		if (input->PushKey(DIK_S)) {
			transform.translate -= velocity;
		}
		velocity = { speed, 0.0f, 0.0f };
		velocity = TransformNormal(velocity, camera->GetWorldMatrix());
		if (input->PushKey(DIK_A)) {
			transform.translate -= velocity;
		}
		if (input->PushKey(DIK_D)) {
			transform.translate += velocity;
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
