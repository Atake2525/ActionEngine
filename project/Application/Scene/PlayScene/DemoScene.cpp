#include "DemoScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"



void DemoScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = new Camera();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera);

	grid = new Object3d();
	grid->Initialize();
	grid->SetModel("Resources/Debug", "Grid.obj");
}

void DemoScene::Update() {

	grid->Update();

	camera->Update();

	input->Update();
}

void DemoScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	grid->Draw();

	ParticleManager::GetInstance()->Draw();

}

void DemoScene::Finalize() {

	delete grid;
}