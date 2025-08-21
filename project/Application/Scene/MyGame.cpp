#include "MyGame.h"

void MyGame::Initialize() {

	FrameWork::Initialize();

#pragma region 基盤システムの初期化

	/*WinApp::GetInstance()->Initialize(1920, 1080, WindowMode::FullScreen, L"Engine");*/
	WinApp::GetInstance()->Initialize();

	directxBase = new DirectXBase();
	directxBase->Initialize();

	SrvManager::GetInstance()->Initialize(directxBase);

	directxBase->InitializePosteffect();

	ImGuiManager::GetInstance()->Initialize(directxBase);

	SpriteBase::GetInstance()->Initialize(directxBase);

	Object3dBase::GetInstance()->Initialize(directxBase);

	SkinningObject3dBase::GetInstance()->Initialize(directxBase);

	WireFrameObjectBase::GetInstance()->Initialize(directxBase);

	SkyBox::GetInstance()->Initialize(directxBase);

	ModelBase::GetInstance()->Initialize(directxBase);

	TextureManager::GetInstance()->Initialize(directxBase);

	ModelManager::GetInstance()->Initialize(directxBase);

	ParticleManager::GetInstance()->Initialize(directxBase);

	CollisionManager::GetInstance()->Initialize();

	JsonLoader::GetInstance()->Initialize();

	Light::GetInstance()->Initialize(directxBase);

	Input::GetInstance()->Initialize();

	Audio::GetInstance()->Initialize();

	//// ↓---- シーンの初期化 ----↓ ////

	SceneManager::GetInstance();
	
	SceneManager::GetInstance()->SetNextScene("GAMESCENE");

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

	directxBase->Update();
	Light::GetInstance()->Update();
	SceneManager::GetInstance()->Update();
	ParticleManager::GetInstance()->Update();
	//JsonLoader::GetInstance()->Update();

	if (SceneManager::GetInstance()->EndRequest())
	{
		finished = true;
	}
}

void MyGame::Draw() {

	// ImGuiの内部コマンドを生成する
	ImGui::Render();

	directxBase->PreDrawRenderTexture();

	//gameScene->Draw();
	SkyBox::GetInstance()->Draw();
	SceneManager::GetInstance()->Draw();

	directxBase->PostDrawRenderTexture();

	directxBase->PreDraw();

	ParticleManager::GetInstance()->Draw();

	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directxBase->GetCommandList().Get());

	directxBase->PostDraw();
}

void MyGame::Finalize() {

	WinApp::GetInstance()->Finalize();

	directxBase->Finalize();
	delete directxBase;

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

	//// ↓---- シーンの解放 ----↓ ////

	//SceneManager::GetInstance()->Finalize();

	//// ↑---- シーンの解放 ----↑ ////

	FrameWork::Finalize();
}