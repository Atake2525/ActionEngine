#include "TestScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"


void TestScene::Initialize() {

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

	grid = new Object3d();
	grid->Initialize();
	grid->SetModel("Resources/Debug", "Grid.obj");

	box1 = std::make_unique<Object3d>();
	box1->Initialize();
	box1->SetModel("Resources/Debug/gltf", "hunmer.gltf", true);
	box1->SetTranslate({ 5.0f, 0.0f, 5.0f });

	box2 = std::make_unique<Object3d>();
	box2->Initialize();
	box2->SetModel("Resources/Debug/gltf", "Box.gltf", true);
	box2->SetTranslate({ 0.0f, 0.0f, 5.0f });
}

void TestScene::Update() {

	grid->Update();

	camera->Update();

	if (input->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}

	SkyBox::GetInstance()->Update();

	Transform t = box1->GetTransform();
	AABB aabb = box1->GetAABB();
	ImGui::Begin("Box");
	ImGui::DragFloat3("Translate", &t.translate.x, 0.1f);
	ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &t.rotate.x, SwapRadian(1.0f));
	ImGui::DragFloat3("MIN", &aabb.min.x, 0.0f);
	ImGui::DragFloat3("MAX", &aabb.max.x, 0.0f);
	ImGui::End();

	box1->SetTransform(t);
	box1->Update();
	box2->Update();

	bool flag = false;

	for (const OBB obb : box1->GetMultiMeshOBB())
	{
		if (CheckOBBCollision(obb, box2->GetOBB()))
		{
			flag = true;
		}
	}

	//if (CheckOBBCollision(box1->GetOBB(), box2->GetOBB()))
	//{
	//	flag = true;
	//}

	if (flag)
	{
		box1->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	}
	else
	{
		box1->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	input->Update();
}

void TestScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	box1->Draw();
	box2->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	grid->Draw();

	ParticleManager::GetInstance()->Draw();

}

void TestScene::Finalize() {

	delete camera;

	delete grid;
}