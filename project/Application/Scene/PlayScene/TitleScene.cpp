#include "TitleScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "WinApp.h"


void TitleScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = new Camera();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera);
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera);

	ParticleManager::GetInstance()->SetCamera(camera);

	playerModel = new Object3d();
	playerModel->Initialize();
	playerModel->SetModel("Resources/Model/gltf/char", "idle.gltf", true, true);
	playerModel->ToggleStartAnimation();
	playerModel->SetTranslate({ 0.0f, 0.1f, 0.0f });

	stageModel = new Object3d();
	stageModel->Initialize();
	stageModel->SetModel("Resources/Debug/gltf", "LandPlate.gltf", true);

	stageUI = new UI();
	stageUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) }, Origin::Center, "Resources/Sprite/start.png");

	startUI = new UI();
	exitUI = new UI();
	

}

void TitleScene::Update() {

	switch (select)
	{
	case TitleScene::Select::Start:

		break;
	case TitleScene::Select::Stage:

		break;
	case TitleScene::Select::Setting:

		break;
	case TitleScene::Select::Exit:

		break;
	}

	stageModel->Update();

	playerModel->Update();

	camera->Update();

	if (stageUI->OnButton())
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}

	SkyBox::GetInstance()->Update();

	input->Update();
}

void TitleScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();



	Object3dBase::GetInstance()->ShaderDraw();

	stageModel->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	playerModel->Draw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();


	ParticleManager::GetInstance()->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

	stageUI->Draw();

}

void TitleScene::Finalize() {

	delete stageModel;

	delete camera;

	delete playerModel;

	delete startUI;

	delete stageUI;

	delete exitUI;
}