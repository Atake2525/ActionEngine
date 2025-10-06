#include "TestScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "CollisionManager.h"
#include "Collision.h"

void TestScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = std::make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());

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

	plate = std::make_unique<Object3d>();
	plate->Initialize();
	//plate->SetModel("Resources/Debug/gltf", "Plante.gltf", true);
	plate->SetModel("Resources/Model/gltf/Stage", "dash.gltf", true);

	CollisionManager::GetInstance()->AddCollision(plate.get(), "plate");

	player = std::make_unique<Player>();
	Transform t = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
	};
	player->Initialize(camera.get(), input, t, true);

	//LevelData jsonData = JsonLoader::GetInstance()->LoadJsonTransform("Resources/Stage/Json", "Debug.json");

	/*for (const auto& data : jsonData.datas)
	{
		if (data.second.file_name == "Trap")
		{

		}
	}*/

}

void TestScene::Update() {

	player->Update();
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

	if (CheckOBBCollision(box1->GetOBB(), box2->GetOBB()))
	{
		flag = true;
	}

	if (CheckOBBCollision(box1->GetOBB(), player->GetOBB()))
	{
		flag = true;
	}

	if (flag)
	{
		box1->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	}
	else
	{
		box1->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	if (input->TriggerKey(DIK_ESCAPE))
	{
		finished = true;
	}

	if (input->TriggerKey(DIK_F11))
	{
		cursorshow = !cursorshow;
	}
	input->ShowMouseCursor(cursorshow);

	plate->Update();

	input->Update();
}

void TestScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	box1->Draw();
	box2->Draw();
	plate->Draw();
	//player->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();


	WireFrameObjectBase::GetInstance()->ShaderDraw();

	grid->Draw();

	ParticleManager::GetInstance()->Draw();

}

void TestScene::Finalize() {

	delete grid;

	CollisionManager::GetInstance()->DeleteCollision("plate");
}