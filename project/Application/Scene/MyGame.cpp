#include "MyGame.h"

void MyGame::Initialize() {

	FrameWork::Initialize();

#pragma region 基盤システムの初期化

	WinApp::GetInstance()->Initialize(1920, 1080, WindowMode::FullScreen, L"Engine");
	//WinApp::GetInstance()->Initialize();

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

	SceneManager::GetInstance();
	
	SceneManager::GetInstance()->SetNextScene("TEST");

	//gameScene->Initialize();

	//// ↑---- シーンの初期化 ----↑ ////
}

void MyGame::Update() {
	FrameWork::Update();

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

	FadeManager::GetInstance()->Finalize();

	//// ↓---- シーンの解放 ----↓ ////

	//SceneManager::GetInstance()->Finalize();

	//// ↑---- シーンの解放 ----↑ ////

	FrameWork::Finalize();
}