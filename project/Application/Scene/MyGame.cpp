#include "MyGame.h"

using namespace ActionEngine::Stage;

namespace {

void FitSceneRenderAreaToAspect(float area[4], float aspectRatio, bool fitWidth)
{
	if (aspectRatio <= 0.0f)
	{
		return;
	}

	const float clientWidth = float(WinApp::GetInstance()->GetkClientWidth());
	const float clientHeight = float(WinApp::GetInstance()->GetkClientHeight());
	const float maxWidth = std::max(1.0f, clientWidth - area[0]);
	const float maxHeight = std::max(1.0f, clientHeight - area[1]);

	if (fitWidth)
	{
		area[2] = std::clamp(area[2], 1.0f, maxWidth);
		area[3] = area[2] / aspectRatio;
		if (area[3] > maxHeight)
		{
			area[3] = maxHeight;
			area[2] = area[3] * aspectRatio;
		}
	}
	else
	{
		area[3] = std::clamp(area[3], 1.0f, maxHeight);
		area[2] = area[3] * aspectRatio;
		if (area[2] > maxWidth)
		{
			area[2] = maxWidth;
			area[3] = area[2] / aspectRatio;
		}
	}
}

}

void MyGame::Initialize() {

	FrameWork::Initialize();

#pragma region 基盤システムの初期化

	GameTime::GetInstance()->Initialize();

#ifndef NDEBUG
	WinApp::GetInstance()->Initialize();
#else
	WinApp::GetInstance()->Initialize(1920, 1080, WindowMode::FullScreen, L"走快");
#endif // !NDEBUG


	DirectXBase::GetInstance()->Initialize();

	SrvManager::GetInstance()->Initialize();

	DirectXBase::GetInstance()->InitializePosteffect();

	ImGuiManager::GetInstance()->Initialize();

	Render2DBase::GetInstance()->Initialize();

	Object3dBase::GetInstance()->Initialize();

	SkinningObject3dBase::GetInstance()->Initialize();

	WireFrameObjectBase::GetInstance()->Initialize();

	DebugLineBase::GetInstance()->Initialize();

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
	Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select");
	Audio::GetInstance()->LoadMP3("Resources/sound/enter.mp3", "select_enter");
	Audio::GetInstance()->LoadMP3("Resources/sound/cancel.mp3", "select_cancel");

	FadeManager::GetInstance()->Initialize();

	//// ↓---- シーンの初期化 ----↓ ////

	StageCount::GetInstance()->Initialize();

	SceneManager::GetInstance();
	
	SceneManager::GetInstance()->SetNextScene("TITLE");

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
#ifndef NDEBUG
	ImGuiManager::GetInstance()->BeginDockSpace();
#endif // !NDEBUG

#ifndef NDEBUG
	static bool sceneRenderAreaInitialized = false;
	static float sceneRenderArea[4] = {};
	static bool keepSceneAspect = true;
	static int sceneAspectFitMode = 0;
	static float sceneAspect[2] = { 16.0f, 9.0f };
	if (!sceneRenderAreaInitialized)
	{
		const float clientWidth = float(WinApp::GetInstance()->GetkClientWidth());
		const float clientHeight = float(WinApp::GetInstance()->GetkClientHeight());
		sceneRenderArea[0] = clientWidth > 600.0f ? 300.0f : 0.0f;
		sceneRenderArea[1] = 0.0f;
		sceneRenderArea[2] = clientWidth > 600.0f ? clientWidth - 600.0f : clientWidth;
		sceneRenderArea[3] = clientHeight;
		sceneRenderAreaInitialized = true;
	}
	if (keepSceneAspect)
	{
		FitSceneRenderAreaToAspect(sceneRenderArea, sceneAspect[0] / sceneAspect[1], sceneAspectFitMode == 0);
	}
	DirectXBase::GetInstance()->SetSceneRenderArea(sceneRenderArea[0], sceneRenderArea[1], sceneRenderArea[2], sceneRenderArea[3]);
#else
	DirectXBase::GetInstance()->SetSceneRenderArea(0.0f, 0.0f, float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()));
#endif // !NDEBUG

	DirectXBase::GetInstance()->Update();
#ifndef NDEBUG
	GameTime::GetInstance()->DrawImGui();
#endif // !NDEBUG
	Light::GetInstance()->Update();
	/*if (WinApp::GetInstance()->IsWindowActive())
	{
		SceneManager::GetInstance()->Update();
	}*/
	SceneManager::GetInstance()->Update();
	ParticleManager::GetInstance()->Update();
	Audio::GetInstance()->Update();
	FadeManager::GetInstance()->Update();

#ifndef NDEBUG

	ImGui::SetNextWindowPos(ImVec2{ float(WinApp::GetInstance()->GetkClientWidth()) - 300.0f, 128.0f * 1 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2{ 300.0f, 245.0f }, ImGuiCond_FirstUseEver);
	ImGui::Begin("シーン");
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
	if (ImGui::Button("Reset SceneArea", { 280, 24 }))
	{
		const float clientWidth = float(WinApp::GetInstance()->GetkClientWidth());
		const float clientHeight = float(WinApp::GetInstance()->GetkClientHeight());
		sceneRenderArea[0] = clientWidth > 600.0f ? 300.0f : 0.0f;
		sceneRenderArea[1] = 0.0f;
		sceneRenderArea[2] = clientWidth > 600.0f ? clientWidth - 600.0f : clientWidth;
		sceneRenderArea[3] = clientHeight;
	}
	ImGui::Checkbox("Keep Aspect", &keepSceneAspect);
	ImGui::DragFloat2("Aspect W/H", sceneAspect, 0.1f, 1.0f, 100.0f);
	ImGui::RadioButton("Fit Width", &sceneAspectFitMode, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Fit Height", &sceneAspectFitMode, 1);
	ImGui::DragFloat2("Scene Pos", &sceneRenderArea[0], 1.0f, 0.0f);
	ImGui::DragFloat2("Scene Size", &sceneRenderArea[2], 1.0f, 1.0f);
	if (keepSceneAspect)
	{
		FitSceneRenderAreaToAspect(sceneRenderArea, sceneAspect[0] / sceneAspect[1], sceneAspectFitMode == 0);
	}
	DirectXBase::GetInstance()->SetSceneRenderArea(sceneRenderArea[0], sceneRenderArea[1], sceneRenderArea[2], sceneRenderArea[3]);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2{ float(WinApp::GetInstance()->GetkClientWidth()) - 300.0f, 128.0f + 245.0f }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2{ 300.0f, 82.5f }, ImGuiCond_FirstUseEver);
	ImGui::Begin("パフォーマンス");
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

	DirectXBase::GetInstance()->ApplyFullViewport();

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

	Render2DBase::GetInstance()->Finalize();

	Object3dBase::GetInstance()->Finalize();

	SkinningObject3dBase::GetInstance()->Finalize();

	WireFrameObjectBase::GetInstance()->Finalize();

	DebugLineBase::GetInstance()->Finalize();

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

	FrameWork::Finalize();
}
