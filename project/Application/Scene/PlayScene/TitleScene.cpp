#include "TitleScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "WinApp.h"
#include "GameTime.h"
#include "EasingUtility.h"

using namespace std;

void TitleScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -4.4f });

	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());

	title = make_unique<Object3d>();
	title->Initialize();
	title->SetModel("Resources/Model/gltf/title", "title.gltf", true);
	title->SetTranslate({ -0.04f, 2.0f, 0.0f });
	title->SetRotate(Vector3(SwapRadian(10.0f), 0.01f, 0.0f));

	playerModel = make_unique<Object3d>();
	playerModel->Initialize();
	playerModel->SetModel("Resources/Model/gltf/char", "idle.gltf", true, true);
	playerModel->ToggleStartAnimation();
	playerModel->SetTranslate({ 0.0f, 0.1f, 0.0f });

	stageModel = make_unique<Object3d>();
	stageModel->Initialize();
	stageModel->SetModel("Resources/Model/obj/Stage/map01", "map01.obj", true);

	startUI = make_unique<UI>();
	startUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) - 64.0f * 3.0f }, Origin::Center, "Resources/Sprite/UI/start.png");
	startUI->function = [this]() {
		start = true;
	};

	playUI = make_unique<UI>();
	playUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f) + 128.0f, float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) }, Origin::Center, "Resources/Sprite/UI/play.png");
	playUI->function = []() {
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	};

	settingUI = make_unique<UI>();
	settingUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f) + 128.0f, float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) + 72.0f }, Origin::Center, "Resources/Sprite/UI/setting.png");
	settingUI->function = [this]() {
	};

	exitUI = make_unique<UI>();
	exitUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f) + 128.0f, float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) + 72.0f * 2.0f }, Origin::Center, "Resources/Sprite/UI/exit.png");
	exitUI->function = [this]() {
		finished = true;
	};

	creditUI = make_unique<UI>();
	creditUI->CreateButton({ 64.0f + 16.0f, float(WinApp::GetInstance()->GetkClientHeight() - 24.0f - 16.0f) }, Origin::Center, "Resources/Sprite/UI/credit.png");
	creditUI->function = [this]() {
		showCredit = !showCredit;
	};

	uiFrame = make_unique<Sprite>();
	uiFrame->Initialize("Resources/Sprite/UI/uiFrame.png");
	uiFrame->SetAnchorPoint({ 0.5f, 0.5f });
	uiFrame->SetPosition({ startUI->GetTransform().translate.x, startUI->GetTransform().translate.y });

	gamePad = make_unique<Sprite>();
	gamePad->Initialize("Resources/Sprite/UI/gamepad.png");
	//gamePad->SetAnchorPoint({ 0.5f, 0.5f });
	gamePad->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

	gamePadOnFrame = make_unique<Sprite>();
	gamePadOnFrame->Initialize("Resources/Sprite/UI/gamepadONFrame.png");
	//gamePadOnFrame->SetAnchorPoint({ 0.5f, 0.5f });
	gamePadOnFrame->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

	credit_sound = make_unique<Sprite>();
	credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
	credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
	credit_sound->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) });

	Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select", 1.0f);
	Audio::GetInstance()->LoadMP3("Resources/sound/enter.mp3", "enter", 1.0f);
	Audio::GetInstance()->LoadMP3("Resources/sound/Experimenta_Model_short.mp3", "bgm", 0.2f);

	Audio::GetInstance()->Play("bgm", true);

	Audio::GetInstance()->SetMasterVolume(0.0f);
	FadeManager::GetInstance()->FadeIn(1.0f);
}

void TitleScene::Update() {

	if (start && !FadeManager::GetInstance()->IsFade())
	{
		Vector3 position;

		if (input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN) || input->TriggerXButton(DPad::Down))
		{
			int selectNum = static_cast<int>(select);
			selectNum++;
			if (selectNum > maxSelectNum)
			{
				selectNum = 0;
			}
			select = static_cast<Select>(selectNum);
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}
		if (input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP) || input->TriggerXButton(DPad::Up))
		{
			int selectNum = static_cast<int>(select);
			selectNum--;
			if (selectNum < 0)
			{
				selectNum = maxSelectNum;
			}
			select = static_cast<Select>(selectNum);
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}
		
		if (select != selectPre)
		{
			Audio::GetInstance()->Play("select");
			switch (select)
			{
			case Select::Play:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = playUI->GetTransform().translate;
				break;
			case Select::Setting:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = settingUI->GetTransform().translate;
				break;
			case Select::Exit:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = exitUI->GetTransform().translate;
				break;
			case Select::Credit:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = creditUI->GetTransform().translate;
				break;
			}
		}

		if (playUI->InCursor() && select != Select::Play)
		{
			select = Select::Play;
			uiFrameStartPoint = uiFrame->GetTransform().translate;
			uiFrameEndPoint = playUI->GetTransform().translate;
			Audio::GetInstance()->Play("select");
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}
		if (settingUI->InCursor() && select != Select::Setting)
		{
			select = Select::Setting;
			uiFrameStartPoint = uiFrame->GetTransform().translate;
			uiFrameEndPoint = settingUI->GetTransform().translate;
			Audio::GetInstance()->Play("select");
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}
		if (exitUI->InCursor() && select != Select::Exit)
		{
			select = Select::Exit;
			uiFrameStartPoint = uiFrame->GetTransform().translate;
			uiFrameEndPoint = exitUI->GetTransform().translate;
			Audio::GetInstance()->Play("select");
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}
		if (creditUI->InCursor() && select != Select::Credit)
		{
			select = Select::Credit;
			uiFrameStartPoint = uiFrame->GetTransform().translate;
			uiFrameEndPoint = creditUI->GetTransform().translate;
			Audio::GetInstance()->Play("select");
			isUIFrameMove = true;
			uiFrameMoveTimer = 0.0f;
		}

		

		if (uiFrameMoveTimer >= 1.0f)
		{
			uiFrameMoveTimer = 0.0f;
			uiFrameStartPoint = { 0.0f, 0.0f };
			uiFrameEndPoint = { 0.0f, 0.0f };
			isUIFrameMove = false;
		}
		if (isUIFrameMove)
		{
			uiFrameMoveTimer += GameTime::GetInstance()->GetDeltaTime() / uiFrameMoveLImitTime;
			position = EaseOutQuint(uiFrameMoveTimer, uiFrameStartPoint, uiFrameEndPoint);
			uiFrame->SetPosition({ position.x, position.y });
		}
		
		if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE) || input->TriggerButton(Controller::A))
		{
			Audio::GetInstance()->Play("enter");
			switch (select)
			{
			case TitleScene::Select::Play:
				FadeManager::GetInstance()->FadeOut(1.0f);
				break;
			case TitleScene::Select::Setting:
				settingUI->TriggerFunction();
				break;
			case TitleScene::Select::Exit:
				exitUI->TriggerFunction();
				break;
			case TitleScene::Select::Credit:
				creditUI->TriggerFunction();
				break;
			}
		}

		if (FadeManager::GetInstance()->CompleteFade())
		{
			playUI->TriggerFunction();
		}

		if (playUI->TriggerOnButton())
		{
			FadeManager::GetInstance()->FadeOut(1.0f);
			Audio::GetInstance()->Play("enter");
		}
		else if (settingUI->TriggerOnButton())
		{
			settingUI->TriggerFunction();
			Audio::GetInstance()->Play("enter");
		}
		else if (exitUI->TriggerOnButton())
		{
			exitUI->TriggerFunction();
			Audio::GetInstance()->Play("enter");
		}
		else if (creditUI->TriggerOnButton())
		{
			creditUI->TriggerFunction();
			Audio::GetInstance()->Play("enter");
		}

		if (playUI->GetButtonOn())
		{
			if (FadeManager::GetInstance()->CompleteFade())
			{
				playUI->TriggerFunction();
			}
		}

	}
	else
	{
		if (startUI->TriggerOnButton() || input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE) || input->TriggerButton(Controller::A))
		{
			startUI->TriggerFunction();
			Audio::GetInstance()->Play("enter");
			uiFrame->SetPosition({ playUI->GetTransform().translate.x, playUI->GetTransform().translate.y });
		}
	}

	if (input->PushKey(DIK_ESCAPE))
	{
		finished = true;
	}

	if (input->IsConnectedController())
	{
		gamePad->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	}
	else
	{
		gamePad->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });
	}

	uiFrame->Update();

	gamePad->Update();

	gamePadOnFrame->Update();

	credit_sound->Update();

	stageModel->Update();

	playerModel->Update();

	camera->Update();

	SkyBox::GetInstance()->Update();

	input->Update();

	Transform titleTransform = title->GetTransform();

	if (titleTransform.translate.y == 2.1f)
	{
		titleUp = false;
	}

	if (titleTransform.translate.y == 1.9f)
	{
		titleUp = true;
	}

	easeTime += GameTime::GetInstance()->GetDeltaTime() / 2.0f;

	if (titleUp)
	{
		titleTransform.translate.y = EaseInOut(easeTime, 1.9f, 2.1f);
	}
	else
	{
		titleTransform.translate.y = EaseInOut(easeTime, 2.1f, 1.9f);
	}

	if (easeTime > 1.0f)
	{
		easeTime = 0.0f;
	}
	title->SetTransform(titleTransform);
	title->Update();

	selectPre = select;

	
}

void TitleScene::Draw() {

	if (start)
	{
		SpriteBase::GetInstance()->ShaderDraw();



		Object3dBase::GetInstance()->ShaderDraw();

		stageModel->Draw();
		title->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		playerModel->Draw();

		WireFrameObjectBase::GetInstance()->ShaderDraw();


		ParticleManager::GetInstance()->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		playUI->Draw();
		settingUI->Draw();
		exitUI->Draw();
		creditUI->Draw();

		uiFrame->Draw();
		gamePad->Draw();
		if (input->IsConnectedController())
		{
			gamePadOnFrame->Draw();
		}

		if (showCredit)
		{
			credit_sound->Draw();
		}
	}
	else
	{
		SpriteBase::GetInstance()->ShaderDraw();



		Object3dBase::GetInstance()->ShaderDraw();

		stageModel->Draw();
		title->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		playerModel->Draw();

		WireFrameObjectBase::GetInstance()->ShaderDraw();


		ParticleManager::GetInstance()->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		startUI->Draw();
		
	}

	SpriteBase::GetInstance()->ShaderDraw();
	uiFrame->Draw();
	//SceneFadeManager::GetInstance()->Draw();

}

void TitleScene::Finalize() {

}