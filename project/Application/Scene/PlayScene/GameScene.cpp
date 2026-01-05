#include "GameScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "GameTime.h"
#include "JsonLoader.h"
#include "GameOver.h"
#include "StageCount.h"

using namespace std;
using namespace ActionEngine::Stage;

void GameScene::Initialize() {
	
	int stageCount = StageCount::GetInstance()->GetStageCount();
	string str = "Resources/Json/Stage/map" + to_string(stageCount) + ".json";
	JsonLoader::GetInstance()->LoadJson(str, "map" + to_string(stageCount), false);

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	camera = make_unique<Camera>();
	camera->SetFarClipDistance(1.0f);
	camera->SetTranslate({ 0.0f, 1.8f, 0.0f });

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();

	gameOver_ = make_unique<GameOver>();
	gameOver_->Initialize();
	gameClear_ = make_unique<GameClearScene>();
	gameClear_->Initialize();

	
	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	player_ = make_unique<ActionPlayer>();
	player_->Initialize(camera.get());
	//player_->Freeze(true);

	land = make_unique<Object3d>();
	land->Initialize();
	land->SetModel("Resources/Model/obj/Stage/map01", "map01_stage.obj", true);
	land->SetDrawHeiht(-1.0f);

	floor = make_unique<Object3d>();
	floor->Initialize();
	floor->SetModel("Resources/Model/obj/Stage/map01", "map01_floor.obj", true);

	CollisionManager::GetInstance()->AddCollision(land.get(), "land");
	CollisionManager::GetInstance()->AddCollision(floor.get(), "floor");

	//LevelData levelData = JsonLoader::GetInstance()->LoadJsonTransform("Resources/Debug/json", "PlayerStartPoint.json");

	goal_ = make_unique<Goal>();
	goal_->Initalize();

	trap_ = make_unique<Trap>();
	trap_->Initialize();
	trap_->SetDrawHeight(0.0f);

	GameTime::GetInstance()->SetDeltaPoint();
	FadeManager::GetInstance()->FadeIn(1.0f);
}

void GameScene::Update() {

#ifndef NDEBUG
	ImGui::Begin("State");
	ImGui::SetWindowPos(ImVec2{ 0.0f, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) });
	float landEnvironment = land->GetEnvironmentCoefficient();
	bool landMetalFlag = land->GetEnableMetallic();
	if (ImGui::TreeNode("環境マップ")) {
		ImGui::DragFloat("land", &landEnvironment, 0.01f);
		ImGui::Checkbox("landメタリック読み込み", &landMetalFlag);
		ImGui::TreePop();
	}
	land->SetEnvironmentCoefficient(landEnvironment);
	land->SetEnableMetallic(landMetalFlag);
	if (ImGui::Button("デバイス更新"))
	{
		input->UpdateDevice();
	}
	ImGui::DragFloat("カメラ速度", &speed, 0.01f);

	float drawHeight = land->GetCullingTemplateData().drawHeight;

	ImGui::DragFloat("カリング高さ", &drawHeight, 0.1f);

	land->SetDrawHeiht(drawHeight);

	ImGui::Checkbox("マウスカーソル表示", &cursorshow);
	if (ImGui::Button("タイトルへ"))
	{
		SceneManager::GetInstance()->SetNextScene("TITLE");
	}
	ImGui::End();

#endif // _DEBUG

	//camera->Update();

	player_->Update();
	land->Update();
	floor->Update();

	//input->Update();

	trap_->Update();

	if (input->TriggerKey(DIK_ESCAPE))
	{
		finished = true;
	}

	if (input->TriggerKey(DIK_F11))
	{
		cursorshow = !cursorshow;
	}
	input->ShowMouseCursor(cursorshow);

	if (input->TriggerKey(DIK_1))
	{
		Audio::GetInstance()->Play2D("bgm", { 0.0f, 0.0f }, false);
	}

	if (!start_)
	{
		if (FadeManager::GetInstance()->CompleteFade())
		{
			start_ = true;
			startMovie_ = true;
			phase_ = 0;
		}
		else
		{
			return;
		}
	}

	if (startMovie_)
	{
		movieTimer_ += GameTime::GetInstance()->GetDeltaTime();

		float farClip = camera->GetFarClipDistance();
		float height;

		AABB landAABB = land->GetAABB();
		Vector3 landSize = landAABB.max - landAABB.min;
		float flatLandSize = landSize.x * landSize.z;

		switch (phase_)
		{
		case 0:

			farClip = Lerp(1.0f, flatLandSize, movieTimer_ / movieTime_);

			camera->SetFarClipDistance(farClip);
			break;
		case 1:
			height = Lerp(-1.0f, landAABB.max.y + 10.0f, movieTimer_ / movieTime_);
			land->SetDrawHeiht(height);
			trap_->SetDrawHeight(height);
			break;
		}

		if (movieTimer_ >= movieTime_)
		{
			if (phase_ == 1)
			{
				startMovie_ = false;
				movieTimer_ = 0.0f;
				player_->Freeze(false);
			}
			else
			{
				camera->SetFarClipDistance(100.0f);
				movieTimer_ = 0.0f;
				phase_++;
				return;
			}
		}
		else
		{
			return;
		}
	}

	goal_->Update(player_->GetAABB());

	if (player_->IsGameOver())
	{
		player_->Freeze(true);
		gameOver_->Update();
	}
	if (goal_->IsGoal())
	{
		player_->Freeze(true);
		gameClear_->Update();
	}

	SkyBox::GetInstance()->Update();
}

void GameScene::Draw() {

		SpriteBase::GetInstance()->ShaderDraw();


		Object3dBase::GetInstance()->ShaderDraw();

		land->Draw();
		floor->Draw();
		goal_->Draw();
		trap_->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		//player_->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		gameOver_->Draw();
		gameClear_->Draw();
}

void GameScene::Finalize() {

	CollisionManager::GetInstance()->DeleteCollision("land");
	CollisionManager::GetInstance()->DeleteCollision("floor");

}