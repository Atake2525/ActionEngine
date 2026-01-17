#include "MyGame.h"

using namespace ActionEngine::Stage;

void MyGame::Initialize() {

	FrameWork::Initialize();

#pragma region 基盤システムの初期化

	GameTime::GetInstance()->Initialize();

	//WinApp::GetInstance()->Initialize(1920, 1080, WindowMode::FullScreen, L"Engine");
	WinApp::GetInstance()->Initialize();

	DirectXBase::GetInstance()->Initialize();

	SrvManager::GetInstance()->Initialize();

	DirectXBase::GetInstance()->InitializePosteffect();

	ImGuiManager::GetInstance()->Initialize();

	SpriteBase::GetInstance()->Initialize();

	Object3dBase::GetInstance()->Initialize();

	SkinningObject3dBase::GetInstance()->Initialize();

	WireFrameObjectBase::GetInstance()->Initialize();

	SkyBox::GetInstance()->Initialize();

	ModelBase::GetInstance()->Initialize();

	TextureManager::GetInstance()->Initialize();

	ModelManager::GetInstance()->Initialize();

	ParticleManager::GetInstance()->Initialize();

	CollisionManager::GetInstance()->Initialize();

	JsonLoader::GetInstance()->Initialize();

	Light::GetInstance()->Initialize();

	Input::GetInstance()->Initialize();

	Audio::GetInstance()->Initialize();

	FadeManager::GetInstance()->Initialize({0.0f, 0.0f, 0.0f});

	//// ↓---- シーンの初期化 ----↓ ////

	StageCount::GetInstance()->Initialize();

	SceneManager::GetInstance();
	
	SceneManager::GetInstance()->SetNextScene("TEST");

	//gameScene->Initialize();

	//// ↑---- シーンの初期化 ----↑ ////

	WinApp::GetInstance()->OpenWindow();
}

void MyGame::Update() {
	FrameWork::Update();

	GameTime::GetInstance()->Update();

	Input::GetInstance()->Update();

	if (WinApp::GetInstance()->ProcessMessage()) {
		finished = true;
	}

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	DirectXBase::GetInstance()->Update();
	Light::GetInstance()->Update();
	SceneManager::GetInstance()->Update();
	ParticleManager::GetInstance()->Update();
	Audio::GetInstance()->Update();
	FadeManager::GetInstance()->Update();
	//JsonLoader::GetInstance()->Update();

#ifndef NDEBUG

	ImGui::Begin("シーン");
	ImGui::SetWindowPos(ImVec2{ float(WinApp::GetInstance()->GetkClientWidth()) - 300.0f, 128.0f * 1 });
	ImGui::SetWindowSize(ImVec2{ 300.0f, 165.0f });
	if (ImGui::Button("TITLE", {280, 40}))
	{
		SceneManager::GetInstance()->SetNextScene("TITLE");
	}
	if (ImGui::Button("GAMESCENE", { 280, 40 }))
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}
	if (ImGui::Button("TEST", { 280, 40 }))
	{
		SceneManager::GetInstance()->SetNextScene("TEST");
	}
	ImGui::End();

	ImGui::Begin("パフォーマンス");
	ImGui::SetWindowPos(ImVec2{ float(WinApp::GetInstance()->GetkClientWidth()) - 300.0f, 128.0f + 165.0f });
	ImGui::SetWindowSize(ImVec2{ 300.0f, 82.5f });
	float fps = 1.0f / GameTime::GetInstance()->GetDeltaTime();
	fps = std::round(fps);
	ImGui::Text("FPS:");
	ImGui::SameLine();
	if (fps > fps * 0.75f)
	{
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "%.1f", fps);
	}
	else if (fps > fps * 0.5f && fps < fps * 0.75f)
	{
		ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "%.1f", fps);
	}
	else
	{
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "%.1f", fps);
	}
	float cpuusage = GameTime::GetInstance()->GetCPUUsagePDH();
	ImGui::Text("CPU使用率:");
	ImGui::SameLine();
	if (cpuusage < 40)
	{
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, "%.1f %%", cpuusage);
	}
	else if (cpuusage > 40 && cpuusage < 79)
	{
		ImGui::TextColored({ 1.0f, 1.0f, 0.0f, 1.0f }, "%.1f %%", cpuusage);
	}
	else
	{
		ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "%.1f %%", cpuusage);
	}
	ImGui::End();	

#endif // _SceneDEBUG

	if (SceneManager::GetInstance()->EndRequest())
	{
		finished = true;
	}
}

void MyGame::Draw() {

	// ImGuiの内部コマンドを生成する
	ImGui::Render();

	DirectXBase::GetInstance()->PreDrawRenderTexture();

	SkyBox::GetInstance()->Draw();
	SceneManager::GetInstance()->Draw();
	FadeManager::GetInstance()->Draw();

	DirectXBase::GetInstance()->PostDrawRenderTexture();

	DirectXBase::GetInstance()->PreDraw();

	ParticleManager::GetInstance()->Draw();

	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectXBase::GetInstance()->GetCommandList().Get());

	DirectXBase::GetInstance()->PostDraw();

	SceneManager::GetInstance()->CallStart();
}

void MyGame::Finalize() {

	WinApp::GetInstance()->Finalize();

	DirectXBase::GetInstance()->Finalize();

	SrvManager::GetInstance()->Finalize();

	ImGuiManager::GetInstance()->Finalize();

	SpriteBase::GetInstance()->Finalize();

	Object3dBase::GetInstance()->Finalize();

	SkinningObject3dBase::GetInstance()->Finalize();

	WireFrameObjectBase::GetInstance()->Finalize();

	SkyBox::GetInstance()->Finalize();

	ModelBase::GetInstance()->Finalize();

	TextureManager::GetInstance()->Finalize();

	ModelManager::GetInstance()->Finalize();

	ParticleManager::GetInstance()->Finalize();

	CollisionManager::GetInstance()->Finalize();

	JsonLoader::GetInstance()->Finalize();

	Light::GetInstance()->Finalize();

	Input::GetInstance()->Finalize();

	Audio::GetInstance()->Finalize();

	GameTime::GetInstance()->Finalize();

	FadeManager::GetInstance()->Finalize();

	//// ↓---- シーンの解放 ----↓ ////

	//SceneManager::GetInstance()->Finalize();

	//// ↑---- シーンの解放 ----↑ ////

	FrameWork::Finalize();
}