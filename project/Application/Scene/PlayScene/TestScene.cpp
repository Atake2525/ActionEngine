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

	actionPlayer = std::make_unique<ActionPlayer>();
	actionPlayer->Initialize(camera.get(), stage->GetJsonName(), false);

	gameOverSprite = std::make_unique<GameOver>();
	gameOverSprite->Initialize();

	//JsonLoader::GetInstance()->LoadJson("Resources/Json/test.json", "test", false);
	JsonLoader::GetInstance()->LoadJson("Resources/Json/wp1.json", "wp1", false);

	goal = make_unique<Goal>();
	goal->Initialize("t");

	capsule = make_unique<Object3d>();
	capsule->Initialize();
	capsule->SetModel("Resources/Model/obj/Player", "PlayerCollision.obj", true);
	capsule->CreateCapsule();
	Capsule capsuleCol = capsule->GetCapsule();
	capsule->SetTranslate({ 5.0f, 35.0f, 0.0f });

	aabbBox = make_unique<Object3d>();
	aabbBox->Initialize();
	aabbBox->SetModel("Resources/Debug/obj", "box.obj", true);

	debugLine = make_unique<DebugLine>();
	debugLine->Initialize();
	debugLine->AddCapsule(Length(capsuleCol.end - capsuleCol.start), capsuleCol.radius, Vector4{ 0.0f, 1.0f, 1.0f, 1.0f });

	Audio::GetInstance()->LoadMP3("Resources/sekiranun.mp3", "bgm", 1.0f);

}

Transform boxTransform = Transform::Default;
void TestScene::Update() {

	/*if (player->IsGameOver())
	{
		player->Freeze(true);
		gameOverSprite->Update();
	}*/

	if (!start_)
	{
		if (FadeManager::GetInstance()->CompleteFade() || !FadeManager::GetInstance()->IsFade())
		{
			start_ = true;
		}
		else
		{
			return;
		}
	}

	player->Update();
	//actionPlayer->Update();
	//camera->Update();

	stage->Update();

	ImGui::Begin("Box");
	ImGui::DragFloat3("translate", &boxTransform.translate.x, 0.1f);
	ImGui::DragFloat3("rotate", &boxTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("scale", &boxTransform.scale.x, 0.1f);
	ImGui::End();

	aabbBox->SetTransform(boxTransform);
	aabbBox->Update();

	capsule->Update();

	debugLine->SetTransform(capsule->GetTransform());
	debugLine->Update();

	if (capsule->CheckCollisionCapsule(aabbBox.get()))
	{
		capsule->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	}
	else
	{
		capsule->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}

	//goal->Update(player->GetAABB());


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

	SkyBox::GetInstance()->Update();
}

void TestScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();

	stage->DrawFrontSprite();

	Object3dBase::GetInstance()->ShaderDraw();

	stage->DrawObject3d();
	capsule->Draw();
	aabbBox->Draw();
	//goal->Draw();
	//player->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	//player->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

	stage->DrawBackSprite();
	gameOverSprite->Draw();

	DebugLineBase::GetInstance()->ShaderDraw();

	debugLine->Draw();

}

void TestScene::Finalize() {
	stage->Finalize();
}