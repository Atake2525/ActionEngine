#include "TestScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "JsonLoader.h"
#include <functional>
#include "Logger.h"
#include "GameTime.h"
#include "StageCount.h"
#include "DebugLineBase.h"
#include "Collision.h"
#include "FadeManager.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;

void TestScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	int stageCount = StageCount::GetInstance()->GetStageCount();
	string str = "Resources/Json/Stage/map" + to_string(stageCount) + ".json";
	JsonLoader::GetInstance()->LoadJson(str, "map" + to_string(stageCount), false);

	camera = std::make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(0.0f), 0.0f, 0.0f));
	//camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

	DebugLineBase::GetInstance()->SetCamera(camera.get());

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());
	ParticleManager::GetInstance()->CreateParticleGroup(ParticleType::plane, "Resources/Particle/circle2.png", "circle");

	stage = std::make_unique<TutorialStage>();
	stage->Initialize();

	
	player = std::make_unique<Player>();
	player->Initialize(camera.get(), stage->GetJsonName());
	camera->Update();

	actionPlayer = std::make_unique<ActionPlayer>();
	actionPlayer->Initialize(camera.get(), stage->GetJsonName(), false);

	gameOverSprite = std::make_unique<GameOver>();
	gameOverSprite->Initialize();

	pause = std::make_unique<Pause>();
	pause->Initialize();

	sprite = std::make_unique<Sprite>();
	sprite->Initialize("Resources/Sprite/R.png");

	//JsonLoader::GetInstance()->LoadJson("Resources/Json/test.json", "test", false);
	JsonLoader::GetInstance()->LoadJson("Resources/Json/wp1.json", "wp1", false);

	box = make_unique<Object3d>();
	box->Initialize();
	box->SetModel("Resources/Debug/obj", "box.obj");
	box->SetTranslate({ 0.0f, 35.0f, 10.0f });

	Audio::GetInstance()->LoadMP3("Resources/sekiranun.mp3", "bgm", 1.0f);

	FadeManager::GetInstance()->FadeIn(2.4f);
}

Transform boxTransform = Transform::Default;
void TestScene::Update() {

	pause->Update();
	if (pause->IsPause())
	{
		return;
	}

	/*if (!start_)
	{
		if (FadeManager::GetInstance()->CompleteFade() || !FadeManager::GetInstance()->IsFade())
		{
			start_ = true;
		}
		else
		{
			return;
		}
	}*/

	player->Update();
	//actionPlayer->Update();

	stage->Update();



	Vector3 penetration = Vector3::Zero;
	if (CollisionCapsuleAABB(player->GetCapsule(), box->GetAABB()))
	{
		box->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		penetration = CapsuleAABBPenetration(player->GetCapsule(), box->GetAABB());
		Vector3 transalte = box->GetTranslate();
		transalte += penetration;
		//box->SetTranslate(transalte);
	}
	else
	{
		box->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
	box->Update();
	//goal->Update(player->GetAABB());

	/*ImGui::Begin("capsule");
	ImGui::DragFloat3("penetration", &penetration.x, 0.1f);
	ImGui::End();*/

	bool flag = false;

	if (input->TriggerKey(DIK_1))
	{
		Audio::GetInstance()->Play3D("bgm", { 0.0f, 0.0f, 0.0f }, false);
	}

	if (input->TriggerKey(DIK_R))
	{
		FadeManager::GetInstance()->FadeOut(0.4f);
		start_ = true;
	}
	if (FadeManager::GetInstance()->CompleteFade() && start_)
	{
		SceneManager::GetInstance()->SetNextScene("TITLE");
	}

	/*if (input->TriggerKey(DIK_ESCAPE))
	{
		finished = true;
	}*/

	if (input->TriggerKey(DIK_F11))
	{
		cursorshow = !cursorshow;
		input->ShowMouseCursor(cursorshow);
	}

	SkyBox::GetInstance()->Update();

	camera->Update();

	sprite->Update();
}

void TestScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();

	stage->DrawFrontSprite();

	Object3dBase::GetInstance()->ShaderDraw();

	stage->DrawObject3d();
	box->Draw();
	//player->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	//player->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

	stage->DrawBackSprite();
	gameOverSprite->Draw();
	pause->Draw();
	sprite->Draw();

	DebugLineBase::GetInstance()->ShaderDraw();

}

void TestScene::Finalize() {
	stage->Finalize();
}