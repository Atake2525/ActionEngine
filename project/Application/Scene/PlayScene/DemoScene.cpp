#include "DemoScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

using namespace std;

void DemoScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/white1x1.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/white1x1.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());
}

void DemoScene::Update() {

	camera->Update();

	if (input->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}

	SkyBox::GetInstance()->Update();

	input->Update();
}

void DemoScene::Draw() {

	Render2DBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	ParticleManager::GetInstance()->Draw();

}

void DemoScene::Finalize() {
}