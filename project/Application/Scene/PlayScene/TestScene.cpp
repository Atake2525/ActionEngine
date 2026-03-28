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
#include "Light.h"

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
	mouseCursor = std::make_unique<MouseCursor>();
	mouseCursor->Initialize("Resources/Sprite/Cursor_Hover.png", "Resources/Sprite/Cursor_Press.png");
	mouseCursor->SetShowCursor(false);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());
	ParticleManager::GetInstance()->CreateParticleGroup(ParticleType::plane, "Resources/Particle/circle2.png", "circle");
	
	player = std::make_unique<Player>();
	stage = std::make_unique<TutorialStage>();
	stage->Initialize(player.get(), camera.get(), mouseCursor.get());
	player->Initialize(camera.get(), stage->GetJsonName());
	camera->Update();


	playerUi = std::make_unique<PlayerUI>();
	playerUi->Initialize(player.get());

	actionPlayer = std::make_unique<ActionPlayer>();
	actionPlayer->Initialize(camera.get(), stage->GetJsonName(), false);


	pause = std::make_unique<Pause>();
	pause->Initialize(mouseCursor.get());

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

	/*for (int i = 0; i < 100000; i++)
	{
		std::unique_ptr<Object3d> obj = std::make_unique<Object3d>();
		obj->Initialize();
		obj->SetModel("Resources/Debug/obj", "box.obj", true);
		obj->SetTranslate({ i * 3.0f, 0.0f, 0.0f });
		boxes.push_back(move(obj));
	}*/
	/*boxes.resize(100000);
	std::function<void()> func1 = [&]() {
		for (int i = 0; i < 30000; i++)
		{
			std::unique_ptr<Object3d> obj = std::make_unique<Object3d>();
			obj->Initialize();
			obj->SetModel("Resources/Debug/obj", "box.obj", true);
			obj->SetTranslate({ i * 3.0f, 0.0f, 0.0f });
			boxes[i] = move(obj);
		}
	};
	std::function<void()> func2 = [&]() {
		for (int i = 0; i < 30000; i++)
		{
			std::unique_ptr<Object3d> obj = std::make_unique<Object3d>();
			obj->Initialize();
			obj->SetModel("Resources/Debug/obj", "box.obj", true);
			obj->SetTranslate({ i * 3.0f, 0.0f, 0.0f });
			boxes[30000 + i] = move(obj);
		}
	};
	std::function<void()> func3 = [&]() {
		for (int i = 0; i < 30000; i++)
		{
			std::unique_ptr<Object3d> obj = std::make_unique<Object3d>();
			obj->Initialize();
			obj->SetModel("Resources/Debug/obj", "box.obj", true);
			obj->SetTranslate({ i * 3.0f, 0.0f, 0.0f });
			boxes[60000 + i] = move(obj);
		}
	};
	std::function<void()> func4 = [&]() {
		for (int i = 0; i < 10000; i++)
		{
			std::unique_ptr<Object3d> obj = std::make_unique<Object3d>();
			obj->Initialize();
			obj->SetModel("Resources/Debug/obj", "box.obj", true);
			obj->SetTranslate({ i * 3.0f, 0.0f, 0.0f });
			boxes[90000 + i] = move(obj);
		}
	};
	std::thread th1(func1);
	std::thread th2(func2);
	std::thread th3(func3);
	std::thread th4(func4);

	th1.join();
	th2.join();
	th3.join();
	th4.join();*/
}

Transform boxTransform = Transform::Default;
void TestScene::Update() {

	if (input->TriggerKey(DIK_ESCAPE))
	{
		finished = true;
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
	playerUi->Update();

	stage->Update();

	for (auto& obj : boxes)
	{
		Vector3 p = obj->GetTranslate();
		p.y += 0.1f;
		obj->SetTranslate(p);
		obj->Update();
	}


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



	if (tim < 100.0f)
	{
		tim += GameTime::GetInstance()->GetDeltaTime() * 50.0f;
	}
	else
	{
		tim = 100.0f;
	}

	if (input->TriggerKey(DIK_R))
	{
		tim = 0.0f;
	}

	camera->SetFarClipDistance(tim);

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
	/*for (auto& obj : boxes)
	{
		obj->Draw();
	}*/

	//player->Draw();

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	//player->Draw();

	SpriteBase::GetInstance()->ShaderDraw();

	stage->DrawBackSprite();
	playerUi->Draw();
	pause->Draw();
	sprite->Draw();

	DebugLineBase::GetInstance()->ShaderDraw();

}

void TestScene::Finalize() {
	stage->Finalize();
}