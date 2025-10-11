#include "GameScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "GameTime.h"
#include "JsonLoader.h"

using namespace std;

void GameScene::Initialize() {

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	camera = make_unique<Camera>();

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(false);

	ParticleManager::GetInstance()->SetCamera(camera.get());

	ParticleManager::GetInstance()->CreateParticleGroupFromOBJ("Resources/Debug/obj", "plane.obj", "plane");

	ParticleManager::GetInstance()->CreateParticleGroup(ParticleType::plane, "Resources/Particle/circle2.png", "circle");

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	Transform pl = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.1f, 0.0f}
	};
	player_ = make_unique<Player>();
	player_->Initialize(camera.get(), input, pl, false);
	player_->SetClearDistance(50.0f);

	land = make_unique<Object3d>();
	land->Initialize();
	//land->SetModel("Resources/Debug/gltf", "LandPlate.gltf", true);
	land->SetModel("Resources/Model/gltf/Stage/map01", "map01.gltf", true);

	CollisionManager::GetInstance()->AddCollision(land.get(), "land");

	Audio::GetInstance()->LoadMP3("Resources/sekiranun.mp3", "bgm", 0.1f);

	LevelData levelData = JsonLoader::GetInstance()->LoadJsonTransform("Resources/Debug/json", "PlayerStartPoint.json");

	goal_ = make_unique<Goal>();
	goal_->Initalize();

}

void GameScene::Update() {

	if (!start)
	{
		GameTime::GetInstance()->SetDeltaPoint();
		FadeManager::GetInstance()->FadeIn(1.0f);
		start = true;
		return;
	}

	cameraTransform = camera->GetTransform();

#ifndef NDEBUG
	ImGui::Begin("State");
	ImGui::SetWindowPos(ImVec2{ 0.0f, 0.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) });
	if (ImGui::TreeNode("Camera")) {
		ImGui::DragFloat3("Tranlate", &cameraTransform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &cameraTransform.scale.x, 0.1f);
		ImGui::TreePop();
	}
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
	ImGuiIO& io = ImGui::GetIO();
	if (io.Framerate > 45)
	{
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "FPS: %.1f", io.Framerate);
	}
	else if (io.Framerate > 30 && io.Framerate < 45)
	{
		ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "FPS: %.1f", io.Framerate);
	}
	else
	{
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "FPS: %.1f", io.Framerate);
	}
	ImGui::Checkbox("マウスカーソル表示", &cursorshow);
	if (ImGui::Button("タイトルへ"))
	{
		SceneManager::GetInstance()->SetNextScene("TITLE");
	}
	ImGui::End();

#endif // _DEBUG

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


	SkyBox::GetInstance()->Update();

	if (isGoal_)
	{
		goal_->Update();
	}
	else
	{
		player_->Update();
		if (player_->IsClear())
		{
			isGoal_ = true;
			Audio::GetInstance()->Stop("bgm");
		}
		camera->Update();
	}

	land->Update();

	input->Update();

}

void GameScene::Draw() {

		SpriteBase::GetInstance()->ShaderDraw();


		Object3dBase::GetInstance()->ShaderDraw();

		land->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		//player_->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		if (isGoal_)
		{
			goal_->Draw();
		}
}

void GameScene::Finalize() {

	CollisionManager::GetInstance()->DeleteCollision("land");

}