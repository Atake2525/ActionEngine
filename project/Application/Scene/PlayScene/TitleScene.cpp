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

	playUI = new UI();
	playUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) }, Origin::Center, "Resources/Sprite/UI/play.png");
	playUI->function = []() {
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	};

	startUI = new UI();
	startUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) }, Origin::Center, "Resources/Sprite/UI/start.png");
	startUI->function = [this]() {
		start = true;
	};

	exitUI = new UI();
	exitUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) + 128.0f * 2.0f }, Origin::Center, "Resources/Sprite/UI/exit.png");
	exitUI->function = [this]() {
		finished = true;
	};

	settingUI = new UI();
	settingUI->CreateButton({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) + 128.0f }, Origin::Center, "Resources/Sprite/UI/setting.png");
	settingUI->function = [this]() {
		return;
	};

	uiFrame = new Sprite();
	uiFrame->Initialize("Resources/Sprite/UI/uiFrame.png");
	uiFrame->SetAnchorPoint({ 0.5f, 0.5f });
	uiFrame->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) });

	gamePad = new Sprite();
	gamePad->Initialize("Resources/Sprite/UI/gamepad.png");
	//gamePad->SetAnchorPoint({ 0.5f, 0.5f });
	gamePad->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

	gamePadOnFrame = new Sprite();
	gamePadOnFrame->Initialize("Resources/Sprite/UI/gamepadONFrame.png");
	//gamePadOnFrame->SetAnchorPoint({ 0.5f, 0.5f });
	gamePadOnFrame->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

	Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select", 0.2f);
}

void TitleScene::Update() {

	if (start)
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
			case TitleScene::Select::Play:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = playUI->GetTransform().translate;
				break;
			case TitleScene::Select::Setting:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = settingUI->GetTransform().translate;
				break;
			case TitleScene::Select::Exit:
				uiFrameStartPoint = uiFrame->GetTransform().translate;
				uiFrameEndPoint = exitUI->GetTransform().translate;
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

		

		if (uiFrameMoveTimer >= 1.0f)
		{
			uiFrameMoveTimer = 0.0f;
			uiFrameStartPoint = { 0.0f, 0.0f };
			uiFrameEndPoint = { 0.0f, 0.0f };
			isUIFrameMove = false;
		}
		if (isUIFrameMove)
		{
			uiFrameMoveTimer += 1.0f / 60.0f / uiFrameMoveLImitTime;
			position = easeOutQuint(uiFrameMoveTimer, uiFrameStartPoint, uiFrameEndPoint);
			uiFrame->SetPosition({ position.x, position.y });
		}
		
		if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE))
		{
			switch (select)
			{
			case TitleScene::Select::Play:
				playUI->TriggerFunction();
				break;
			case TitleScene::Select::Setting:
				settingUI->TriggerFunction();
				break;
			case TitleScene::Select::Exit:
				exitUI->TriggerFunction();
				break;
			}
		}
		if (playUI->OnButton())
		{
			playUI->TriggerFunction();
		}
		else if (settingUI->OnButton())
		{
			settingUI->TriggerFunction();
		}
		else if (exitUI->OnButton())
		{
			exitUI->TriggerFunction();
		}

	}
	else
	{
		if (startUI->OnButton())
		{
			start = true;
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

	stageModel->Update();

	playerModel->Update();

	camera->Update();

	SkyBox::GetInstance()->Update();

	input->Update();

	selectPre = select;
}

void TitleScene::Draw() {

	if (start)
	{
		SpriteBase::GetInstance()->ShaderDraw();



		Object3dBase::GetInstance()->ShaderDraw();

		stageModel->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		playerModel->Draw();

		WireFrameObjectBase::GetInstance()->ShaderDraw();


		ParticleManager::GetInstance()->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		playUI->Draw();
		settingUI->Draw();
		exitUI->Draw();

		uiFrame->Draw();
		gamePad->Draw();
		if (input->IsConnectedController())
		{
			gamePadOnFrame->Draw();
		}

	}
	else
	{
		SpriteBase::GetInstance()->ShaderDraw();

		startUI->Draw();
	}

}

void TitleScene::Finalize() {

	delete stageModel;

	delete camera;

	delete playerModel;

	delete startUI;

	delete playUI;

	delete exitUI;

	delete settingUI;

	delete uiFrame;

	delete gamePad;

	delete gamePadOnFrame;
}