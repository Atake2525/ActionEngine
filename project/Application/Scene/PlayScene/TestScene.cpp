#include "TestScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "CollisionManager.h"
#include "Collision.h"
#include "JsonLoader.h"
#include <functional>
#include "Logger.h"
#include "GameTime.h"
#include "StageCount.h"

using namespace Logger;
using namespace std;

void TestScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	int stageCount = StageCount::GetInstance()->GetStageCount();
	string str = "Resources/Json/Stage/map" + to_string(stageCount) + ".json";
	JsonLoader::GetInstance()->LoadJson(str, "map" + to_string(stageCount), false);

	camera = std::make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(0.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

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

	/*box1 = std::make_unique<Object3d>();
	box1->Initialize();
	box1->SetModel("Resources/Debug/gltf", "trap.gltf", true);
	box1->SetTranslate({ 14.5f, 15.0f, 26.2f });*/

    //CollisionManager::GetInstance()->AddCollision(box1.get(), "box1");

	box2 = std::make_unique<Object3d>();
	box2->Initialize();
	box2->SetModel("Resources/Debug/gltf", "Box.gltf", true);
	box2->SetTranslate({ 0.0f, 0.0f, 5.0f });

	plate = std::make_unique<Object3d>();
	plate->Initialize();
	//plate->SetModel("Resources/Debug/gltf", "Plante.gltf", true);
	plate->SetModel("Resources/Model/obj/Stage/map01", "map01.obj", true);
	//plate->SetEnableMetallic(true);

	CollisionManager::GetInstance()->AddCollision(plate.get(), "plate");
	//CollisionManager::GetInstance()->AddCollision(box2.get(), "box");


	player = std::make_unique<Player>();
	player->Initialize(camera.get(), input, true);

	gameOverSprite = std::make_unique<GameOver>();
	gameOverSprite->Initialize();

	JsonLoader::GetInstance()->LoadJson("Resources/Json/test.json", "test", false);

	trap = std::make_unique<Trap>();
	trap->Initialize("test");

	goal = make_unique<Goal>();
	goal->Initalize();

	Audio::GetInstance()->LoadMP3("Resources/sekiranun.mp3", "bgm", 1.0f);

}

void TestScene::Update() {

	if (player->IsGameOver())
	{
		player->Freeze(true);
		gameOverSprite->Update();
	}

	grid->Update();

	camera->Update();

	if (input->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}

	SkyBox::GetInstance()->Update();

	Transform t = box2->GetTransform();
	AABB aabb = box2->GetAABB();
	ImGui::Begin("Box");
	ImGui::DragFloat3("Translate", &t.translate.x, 0.1f);
	ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
	ImGui::DragFloat3("Rotate", &t.rotate.x, SwapRadian(1.0f));
	ImGui::DragFloat3("MIN", &aabb.min.x, 0.0f);
	ImGui::DragFloat3("MAX", &aabb.max.x, 0.0f);
	ImGui::End();

	/*t.rotate.z += SwapRadian(4.0f);

	box1->SetTransform(t);
	box1->Update();*/
	box2->SetTransform(t);
	box2->Update();

	trap->Update();
	player->Update();

	goal->Update(player->GetAABB());


	bool flag = false;

	if (input->TriggerKey(DIK_1))
	{
		Audio::GetInstance()->Play3D("bgm", { 0.0f, 0.0f, 0.0f }, false);
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

	//box1->Draw();
	//box2->Draw();
	plate->Draw();
	trap->Draw();
	goal->Draw();
	//player->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	//player->Draw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	grid->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

	gameOverSprite->Draw();

	ParticleManager::GetInstance()->Draw();

}

void TestScene::Finalize() {

	delete grid;

	CollisionManager::GetInstance()->DeleteCollision("plate");
	CollisionManager::GetInstance()->DeleteCollision("box");
}