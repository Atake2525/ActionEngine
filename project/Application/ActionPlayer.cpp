#include "ActionPlayer.h"
#include "kMath.h"
#include "ImGuiManager.h"
#include "CollisionManager.h"
#include "WinApp.h"
#include "EasingUtility.h"
#include "GameTime.h"
#include "Light.h"
#include "JsonLoader.h"
#include "StageCount.h"

using namespace std;
using namespace ActionEngine::Stage;

ActionPlayer::~ActionPlayer()
{
	CollisionManager::GetInstance()->DeleteCollisionTarget("player");
}

void ActionPlayer::Initialize(Camera* camera, string jsonName, const bool DebugMode)
{
	debugMode_ = DebugMode;
	this->camera = camera;
	fovY_ = this->camera->GetfovY();
	this->input = Input::GetInstance();
	parent_ = !DebugMode;

	playerTransform_.scale = Vector3::Zero;
	playerTransform_.rotate = Vector3::Zero;
	playerTransform_.translate = { 0.0f, 2.0f, 0.0f };
	if (!DebugMode)
	{
		this->input->ShowMouseCursor(false);
	}

	Transform startPoint = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.1f, 0.0f}
	};
	// スタート地点の取得
	m_jsonName = jsonName;
	vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(jsonName, "startpoint");
	// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
	if (data.size() != 0)
	{
		startPoint = data[0].transform;
	}
	playerTransform_ = startPoint;

	moveVelocity_ = { 0.0f, 0.0f, 0.0f };

	playerModel_ = std::make_unique<Object3d>();
	playerModel_->Initialize();
	playerModel_->SetModel("Resources/Model/gltf/char", "onlyBodyIdle.gltf", true, true);
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk.gltf", "walk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "sneak.gltf", "sneak");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "dash.gltf", "dash");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "jump.gltf", "jump");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "crouch.gltf", "crouch");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "walk_back.gltf", "backwalk");
	playerModel_->AddAnimation("Resources/Model/gltf/char", "fall.gltf", "fall");
	playerModel_->SetTransform(playerTransform_);
	playerModel_->ToggleStartAnimation();

	playerCollisionModel_ = std::make_unique<Object3d>();
	playerCollisionModel_->Initialize();
	playerCollisionModel_->SetModel("Resources/Model/gltf/Player", "PlayerCollision.gltf", true);

	playerAABB_ = playerCollisionModel_->GetAABB();

	CollisionManager::GetInstance()->AddCollisionTarget(playerAABB_, "player");
}

void ActionPlayer::Update() {
	playerTransform_ = playerModel_->GetTransform();
	playerAABB_ = playerCollisionModel_->GetAABB();
	cameraTransform = camera->GetTransform();

	if (parent_)
	{
		// プレイヤーの回転からcameraOffsetを計算してparent
		Vector3 position;
		position = playerTransform_.translate;
		position.y = playerModel_->GetJointPosition("Head").y;
		playerAABB_.max.y = position.y;

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
	if (!freeze_)
	{
		Rotation();
		Move();
	}

	camera->SetRotate(cameraTransform.rotate);

	playerModel_->SetAnimationSpeed(1.0f);
	playerModel_->SetTransform(playerTransform_);
	playerCollisionModel_->SetTransform(playerTransform_);
	playerCollisionModel_->Update();
	playerModel_->Update();

	Light::GetInstance()->SetPositionSpotLight({ playerTransform_.translate.x, playerModel_->GetJointPosition("Head").y, playerTransform_.translate.z });
	Vector3 direction = camera->GetDirection();
	Light::GetInstance()->SetDirectionSpotLight({ direction.x, direction.y, direction.z });

	if (debugMode_)
	{
		DebugUpdate();
	}
}

void ActionPlayer::Draw() {
	playerModel_->Draw();
}

const bool ActionPlayer::IsGameOver() const
{
	if (playerTransform_.translate.y < -10.0f)
	{
		return true;
	}
	return false;
}

void ActionPlayer::Rotation() {
	Vector3 mouseVelocity = input->GetMouseVel3();

	mouseVelocity.x *= cameraSpeed.x / 100.0f;
	mouseVelocity.y *= cameraSpeed.y / 100.0f;


	cameraTransform.rotate.x += mouseVelocity.y;
	cameraTransform.rotate.y += mouseVelocity.x;

	cameraTransform.rotate.x = std::clamp(cameraTransform.rotate.x, SwapRadian(-90.0f), SwapRadian(90.0f));


}

void ActionPlayer::Move()
{
	// 無操作状態ならば何もしないので毎フレームIdle状態にする
	moveType_ = PlayerMoveType::Idle;

	Vector3 penetrationAmount = CollisionManager::GetInstance()->GetPenetration();

	Vector3 cameraDirection = camera->GetDirection();

	// 壁走りの処理
	if (input->PressMouse(1) && (penetrationAmount.x != 0.0f || penetrationAmount.z != 0.0f) && moveVelocity_.y < 0.0f)
	{
		if ((cameraDirection.x < -0.5f || cameraDirection.x > 0.5f) && penetrationAmount.z != 0.0f)
		{
			wallDash_ = true;
			if (penetrationAmount.z < 0.0f)
			{
				if (wallDashRotateEnd_ != wallDashAngle_ * Sign(cameraDirection.x))
				{
					wallDashRotateTimer_ = 0.0f;
					wallDashRotateStart_ = camera->GetRotate().z;
					wallDashRotateEnd_ = wallDashAngle_ * Sign(cameraDirection.x);
				}
				wallDashRotateTimer_ += GameTime::GetInstance()->GetDeltaTime() / wallDashRotateTime_;
				wallDashRotateTimer_ = std::clamp(wallDashRotateTimer_, 0.0f, 1.0f);
			}
			else if (penetrationAmount.z > 0.0f)
			{
				if (wallDashRotateEnd_ != -wallDashAngle_ * Sign(cameraDirection.x))
				{
					wallDashRotateTimer_ = 0.0f;
					wallDashRotateStart_ = camera->GetRotate().z;
					wallDashRotateEnd_ = -wallDashAngle_ * Sign(cameraDirection.x);
				}
				wallDashRotateTimer_ += GameTime::GetInstance()->GetDeltaTime() / wallDashRotateTime_;
				wallDashRotateTimer_ = std::clamp(wallDashRotateTimer_, 0.0f, 1.0f);
			}
			jump_ = false;
			moveVelocity_.y = wallDashAcceleration_;
			speed_.y = wallDashAcceleration_;
		}
		else if ((cameraDirection.z < -0.5f || cameraDirection.z > 0.5f) && penetrationAmount.x != 0.0f)
		{
			wallDash_ = true;
			if (penetrationAmount.x < 0.0f)
			{
				if (wallDashRotateEnd_ != -wallDashAngle_ * Sign(cameraDirection.z))
				{
					wallDashRotateTimer_ = 0.0f;
					wallDashRotateStart_ = camera->GetRotate().z;
					wallDashRotateEnd_ = -wallDashAngle_ * Sign(cameraDirection.z);
				}
				wallDashRotateTimer_ += GameTime::GetInstance()->GetDeltaTime() / wallDashRotateTime_;
				wallDashRotateTimer_ = std::clamp(wallDashRotateTimer_, 0.0f, 1.0f);
			}
			else if (penetrationAmount.x > 0.0f)
			{
				if (wallDashRotateEnd_ != wallDashAngle_ * Sign(cameraDirection.z))
				{
					wallDashRotateTimer_ = 0.0f;
					wallDashRotateStart_ = camera->GetRotate().z;
					wallDashRotateEnd_ = wallDashAngle_ * Sign(cameraDirection.z);
				}
			}
			jump_ = false;
			moveVelocity_.y = wallDashAcceleration_;
			speed_.y = wallDashAcceleration_;
		}
		else if (CollisionManager::GetInstance()->GetGroundDistance("player") >= -2.0f)
		{
			wallDash_ = true;
			jump_ = false;
			moveVelocity_.y = wallDashAcceleration_;
			speed_.y = wallDashAcceleration_;
		}
		else
		{
			if (wallDashRotateEnd_ != 0.0f)
			{
				wallDashRotateTimer_ = 0.0f;
				wallDashRotateStart_ = camera->GetRotate().z;
				wallDashRotateEnd_ = 0.0f;
			}
			wallDash_ = false;
		}
	}
	else if (wallDash_)
	{
		if (wallDashRotateEnd_ != 0.0f)
		{
			wallDashRotateTimer_ = 0.0f;
			wallDashRotateStart_ = camera->GetRotate().z;
			wallDashRotateEnd_ = 0.0f;
		}
		moveType_ = PlayerMoveType::Dash;
		wallDash_ = false;
	}

	if (wallDashRotateEnd_ != camera->GetRotate().z)
	{
		isWallDashRotating_ = true;
	}
	else
	{
		isWallDashRotating_ = false;
	}
	if (isWallDashRotating_)
	{
		wallDashRotateTimer_ += GameTime::GetInstance()->GetDeltaTime() / wallDashRotateTime_;
		wallDashRotateTimer_ = std::clamp(wallDashRotateTimer_, 0.0f, 1.0f);
		cameraTransform.rotate.z = EaseOutExpo(wallDashRotateTimer_, wallDashRotateStart_, wallDashRotateEnd_);
		//camera->SetRotate({ 0.0f ,0.0f, EaseOutExpo(wallDashRotateTimer_, wallDashRotateStart_, wallDashRotateEnd_) });
	}


	// 地面との高さを求めて落下処理を行う
	float dist = CollisionManager::GetInstance()->GetGroundDistance("player");
	if (dist < -0.2f)
	{
		dist = CollisionManager::GetInstance()->GetGroundMAXDistance("player");
	}
	if (dist > 0.0f && !wallDash_)
	{
		jump_ = true;
	}
	// 一番最初にジャンプ状態の有無を調べる(ジャンプ中か否かで移動系処理が変わるため)
	if (input->PushKey(DIK_SPACE))
	{
		if (!jump_)
		{
			if (wallDash_)
			{
				if (penetrationAmount.x > 0.0f)
				{
					speed_.y = jumpAcceleration_;
					speed_.x = -speedLimit_ * Sign(cameraDirection.z);
				}
				else if (penetrationAmount.x < 0.0f)
				{
					speed_.y = jumpAcceleration_;
					speed_.x = speedLimit_ * Sign(cameraDirection.z);
				}
				else if (penetrationAmount.z > 0.0f)
				{
					speed_.y = jumpAcceleration_;
					speed_.x = speedLimit_ * Sign(cameraDirection.x);
				}
				else if (penetrationAmount.z < 0.0f)
				{
					speed_.y = jumpAcceleration_;
					speed_.x = -speedLimit_ * Sign(cameraDirection.x);
				}
				else if (CollisionManager::GetInstance()->GetGroundDistance("player") >= -2.0f)
				{
					speed_.y = jumpAcceleration_;
				}
			}
			else
			{
				speed_.y = jumpAcceleration_;
			}
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

	// カメラのY軸回転角度のみを使用（上下の視点は完全に無視）
	float cameraYRotation = cameraTransform.rotate.y;

	// 前後左右の移動を水平面のみで直接計算
	// 前後移動（Z軸）
	Vector3 forward = {
		sinf(cameraYRotation) * speed_.z,
		0.0f,
		cosf(cameraYRotation) * speed_.z
	};

	// 左右移動（X軸）
	Vector3 right = {
		cosf(cameraYRotation) * speed_.x,
		0.0f,
		-sinf(cameraYRotation) * speed_.x
	};

	// 水平移動と垂直移動を組み合わせる
	moveVelocity_.x = forward.x + right.x;
	moveVelocity_.y = speed_.y;  // ジャンプ・落下はそのまま
	moveVelocity_.z = forward.z + right.z;


	if (penetrationAmount.y < 0.0f)
	{
		speed_.y = 0.0f;
		moveVelocity_.y = 0.0f;
		jump_ = false;
	}
	else if (penetrationAmount.y > 0.0f)
	{
		speed_.y = 0.0f;
		moveVelocity_.y = 0.0f;
	}
	// プレイヤーの移動量を今のプレイヤーの位置に加算する
	playerTransform_.translate += moveVelocity_;
	playerAABB_ += moveVelocity_;
	CollisionManager::GetInstance()->UpdateCollisionTarget(playerAABB_, "player");
	CollisionManager::GetInstance()->Update("player");
	penetrationAmount = CollisionManager::GetInstance()->GetPenetration();

	if (penetrationAmount.y > 0.0f)
	{
		playerTransform_.scale.x = 1.0f;
	}

	// オブジェクトに衝突している時のために貫通量を引く
	playerTransform_.translate -= penetrationAmount;
	// プレイヤーの回転をカメラの正面を向くように変える（Y軸回転のみ）
	playerTransform_.rotate.y = cameraTransform.rotate.y;
	// X軸（上下の視点）とZ軸（ロール）は0に固定
	playerTransform_.rotate.x = 0.0f;
	playerTransform_.rotate.z = 0.0f;

	if (penetrationAmount.x != 0.0f || penetrationAmount.z != 0.0f)
	{
		moveType_ = PlayerMoveType::Idle;
	}

	// 速度が歩行状態よりも早ければ速度が上がっている感を出すためにFovを上げる(ジャンプ中はFovが増えないようにする)
	if (((speed_.x == dashSpeed_ * speedLimit_ || speed_.z == dashSpeed_ * speedLimit_) || (speed_.x == -(dashSpeed_ * speedLimit_) || speed_.z == -(dashSpeed_ * speedLimit_))) && !jump_)
	{
		// ダッシュ
		fovTime_ = 0.0f;
		afterFovY_ = normalFovY_ + fovYBoost_;
	}
	else if (((speed_.x <= walkSpeed_ || speed_.z <= walkSpeed_) || (speed_.x <= -walkSpeed_ || speed_.z <= -walkSpeed_)) && !jump_)
	{
		// 歩行
		fovTime_ = 0.0f;
		afterFovY_ = normalFovY_;
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

void ActionPlayer::Sneak()
{
}

void ActionPlayer::DebugUpdate()
{
	Transform transform = camera->GetTransform();
	// ウィンドウ設定
	ImGui::Begin("Player");
	{
		// レイアウト定数 (マジックナンバーを避けるため変数化推奨ですが、元のロジックを維持)
		float headerHeight = 18.0f * 3.0f;
		float clientHeight = float(WinApp::GetInstance()->GetkClientHeight());

		ImGui::SetWindowPos(ImVec2{ 0.0f, headerHeight });
		ImGui::SetWindowSize(ImVec2{ 300.0f, clientHeight - headerHeight });

		// ---------------------------------------------------------
		// General Settings
		// ---------------------------------------------------------
		if (ImGui::CollapsingHeader("General Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Camera Move (操作有効)", &cameraMove_);
			ImGui::Checkbox("Camera Follow (追従)", &parent_);
		}

		// ---------------------------------------------------------
		// Transform
		// ---------------------------------------------------------
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("Position", &playerTransform_.translate.x, 0.1f);
			ImGui::DragFloat3("Rotate", &playerTransform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &playerTransform_.scale.x, 0.1f);
		}

		// ---------------------------------------------------------
		// Physics & Movement
		// ---------------------------------------------------------
		if (ImGui::CollapsingHeader("移動関係"))
		{
			ImGui::TextDisabled("Velocity / Acceleration");
			ImGui::DragFloat3("Move Speed Input", &speed_.x, 0.1f);
			ImGui::DragFloat3("Current Velocity", &moveVelocity_.x, 0.1f);

			ImGui::Separator();

			ImGui::TextDisabled("Jump & Gravity");
			ImGui::DragFloat("Jump Power", &jumpAcceleration_, 0.1f);
			ImGui::DragFloat("Gravity Force", &fallAcceleration_, 0.01f);
			ImGui::DragFloat("Max Fall Speed", &fallLimit_, 0.1f);
		}

		// ---------------------------------------------------------
		// Camera Details
		// ---------------------------------------------------------
		if (ImGui::CollapsingHeader("カメラ関係"))
		{
			ImGui::DragFloat3("Offset", &cameraOffset_.x, 0.1f);

			if (ImGui::TreeNode("FOV Settings"))
			{
				ImGui::DragFloat("Base FOV", &normalFovY_, 0.01f);
				ImGui::DragFloat("FOV Boost", &fovYBoost_, 0.01f);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("ダッシュ / 壁走り"))
			{
				ImGui::DragFloat("Rotate Timer", &wallDashRotateTimer_, 0.01f);
				ImGui::DragFloat("Rotate Time", &wallDashRotateTime_, 0.01f);

				// 計算値の表示のみにするか、編集可能にするか確認が必要ですが、元のままDragFloatにします
				Vector3 camDir = camera->GetDirection();
				float angleX = wallDashAngle_ * Sign(camDir.x);
				float angleZ = wallDashAngle_ * Sign(camDir.z);

				// ※ここは計算結果を表示しているように見えますが、ポインタを渡しているので
				// 変数を書き換える意図がある場合は元の変数を渡す必要があります。
				// 一時変数への操作になるため、表示のみであれば InputFloat ではなく Text 推奨です。
				ImGui::DragFloat("Calc Angle X", &angleX, 0.01f);
				ImGui::DragFloat("Calc Angle Z", &angleZ, 0.01f);

				ImGui::TreePop();
			}
		}

		// ---------------------------------------------------------
		// Debug Info (Read Only)
		// ---------------------------------------------------------
		if (ImGui::CollapsingHeader("Debug Info", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Direction
			Vector3 dir = camera->GetDirection();
			ImGui::Text("Camera Direction:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "[ %.2f, %.2f, %.2f ]", dir.x, dir.y, dir.z);

			ImGui::Bullet();
			ImGui::Text("Orientation: ");
			ImGui::SameLine();
			if (dir.x > 0.1f)  ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Right");
			if (dir.x < -0.1f) ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Left");

			ImGui::SameLine();
			if (dir.z > 0.1f)  ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Forward");
			if (dir.z < -0.1f) ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Back");

			ImGui::Separator();

			// Collision
			float dist = CollisionManager::GetInstance()->GetGroundDistance("player");
			Vector3 pen = CollisionManager::GetInstance()->GetPenetration();

			ImGui::Text("Ground Dist:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "%.2f", dist);

			ImGui::Text("Penetration:");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "X:%.1f Y:%.1f Z:%.1f", pen.x, pen.y, pen.z);
		}
	}
	ImGui::End();


	if (input->TriggerKey(DIK_R))
	{
		Transform startPoint = {
			{1.0f, 1.0f, 1.0f},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.1f, 0.0f}
		};
		vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(m_jsonName, "startpoint");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (data.size() != 0)
		{
			startPoint = data[0].transform;
		}
		playerTransform_ = startPoint;

		moveVelocity_ = { 0.0f, 0.0f, 0.0f };
		moveVelocity_ = { 0.0f };
		playerModel_->SetTransform(playerTransform_);
		speed_ = { 0.0f, 0.0f, 0.0f };
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
