#include "GameScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

#include "JsonLoader.h"

void GameScene::Initialize() {

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	camera = new Camera();

	SkyBox::GetInstance()->SetCamera(camera);
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	ParticleManager::GetInstance()->SetCamera(camera);

	ParticleManager::GetInstance()->CreateParticleGroupFromOBJ("Resources/Debug/obj", "plane.obj", "plane");

	ParticleManager::GetInstance()->CreateParticleGroup(ParticleType::plane, "Resources/Particle/circle2.png", "circle");

	Object3dBase::GetInstance()->SetDefaultCamera(camera);

	Transform pl = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.01f, 0.0f}
	};
	player_ = new Player();
	player_->Initialize(camera, input, pl, true);

	grid = new Object3d();
	grid->Initialize();
	grid->SetModel("Resources/Debug", "Grid.obj");
	grid->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });

	terrain = new Object3d();
	terrain->Initialize();
	terrain->SetModel("Resources/Debug/obj", "terrain.obj");
	terrain->SetTranslate({ 20.0f, 0.01f, 0.0f });
	terrain->SetEnableLighting(true);

	land = new Object3d();
	land->Initialize();
	land->SetModel("Resources/Debug/gltf", "LandPlate.gltf", true);
	//land->SetEnvironmentCoefficient(1.0f);

	CollisionManager::GetInstance()->AddCollision(land, "land");

	Audio::GetInstance()->LoadMP3("Resources/sekiranun.mp3", "bgm", 0.1f);

	LevelData levelData = JsonLoader::GetInstance()->LoadJsonTransform("Resources/Debug/json", "PlayerStartPoint.json");

}

void GameScene::Update() {

	//ImGui::ShowDemoWindow();
	cameraTransform = camera->GetTransform();
	//sprite->Update();

#ifdef _DEBUG
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

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
	//camera->SetTranslateParent(human->GetWorldMatrix());
	camera->Update();

	SkyBox::GetInstance()->Update();

	player_->Update();

	grid->Update();

	terrain->Update();

	land->Update();

	input->Update();

}

void GameScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	terrain->Draw();

	land->Draw();


	SkinningObject3dBase::GetInstance()->ShaderDraw();

	player_->Draw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	grid->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

}

void GameScene::Finalize() {

	delete camera;

	delete player_;

	delete grid;

	delete terrain;

	delete land;

}