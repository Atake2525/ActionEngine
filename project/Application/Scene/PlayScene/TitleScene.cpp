#include "TitleScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "WinApp.h"
#include "GameTime.h"
#include "EasingUtility.h"
#include "Logger.h"

using namespace Logger;
using namespace std;

bool TitleScene::InitializeStep() {
	// ステップごとに重い処理を分割して1フレームごとに呼ぶ
	// initStep の値に応じて処理を行い、最後に true を返す
	switch (initStep)
	{
	case 0:
		// 軽量な初期化 (必須 UI とカメラ)
		camera = make_unique<Camera>();
		camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
		camera->SetTranslate({ 0.0f, 2.8f, -4.4f });

		SkyBox::GetInstance()->SetCamera(camera.get());

		input = Input::GetInstance();
		input->ShowMouseCursor(true);

		Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

		ParticleManager::GetInstance()->SetCamera(camera.get());

		Log("カメラ初期化完了\n");
		// 次ステップへ
		initStep++;
		return false;

	case 1:

		// プレースホルダとなるオブジェクトは最低限生成する
		title = make_unique<Object3d>();
		title->Initialize();
		// モデルの本読み込みは後のステップで行う
		title->SetTranslate({ -0.04f, 2.0f, 0.0f });
		title->SetRotate(Vector3(SwapRadian(10.0f), 0.01f, 0.0f));

		playerModel = make_unique<Object3d>();
		playerModel->Initialize();
		playerModel->SetTranslate({ 0.0f, 0.1f, 0.0f });

		stageModel = make_unique<Object3d>();
		stageModel->Initialize();

		// UIの初期化は軽めなのでここで実行
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
		gamePad->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

		gamePadOnFrame = make_unique<Sprite>();
		gamePadOnFrame->Initialize("Resources/Sprite/UI/gamepadONFrame.png");
		gamePadOnFrame->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

		credit_sound = make_unique<Sprite>();
		credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
		credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
		credit_sound->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) });

		// オーディオ読み込みは次のステップで行う（重いため）

		// 次ステップへ
		/*initStep++;
		return false;*/

	//case 2:
		// テクスチャ等のディスクI/Oを行うステップ
		// ここをワーカースレッドにすることもできるが、まずはフレーム分割で非ブロッキングにする
		TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");
		SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

		Log("UI|skyBox初期化完了\n");
		// 次ステップへ
		initStep++;
		return false;

	case 2:
		// モデル本体の読み込み (比較的重い)
		// ModelManager は内部で同期読み込みだが、分割して呼ぶことでフレームブロックを分散
		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/title", "title.gltf", true);
		title->SetModel("Resources/Model/gltf/title", "title.gltf", true);

		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/char", "idle.gltf", true, true);
		playerModel->SetModel("Resources/Model/gltf/char", "idle.gltf", true, true);
		playerModel->ToggleStartAnimation();

		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/Stage/map01", "map01.gltf", true);
		stageModel->SetModel("Resources/Model/gltf/Stage/map01", "map01.gltf", true);

		Log("モデル初期化完了\n");
		// 次ステップへ
		initStep++;
		return false;

	case 3:
		// オーディオなど残りの読み込み
		Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select", 1.0f);
		Audio::GetInstance()->LoadMP3("Resources/sound/enter.mp3", "enter", 1.0f);
		Audio::GetInstance()->LoadMP3("Resources/sound/Experimenta_Model_short.mp3", "bgm", 0.2f);

		Audio::GetInstance()->Play("bgm", true);
		Audio::GetInstance()->SetMasterVolume(0.0f);

		Log("オーディオ初期化完了\n");
		// 全ての初期化完了
		Log("タイトルシーン初期化処理完了\n");
		assetsLoaded = true;
		initStep++;
		return true;

	default:
		return assetsLoaded;
	}

	
}

void TitleScene::Initialize() {
	// 非ブロッキング初期化を開始する
	initStep = 0;
	assetsLoaded = false;
	// ここでは即座に1フレーム分の軽量初期化だけ行い、残りは Update 内で進める
	// InitializeStep() を一度呼んで UI 等の初期化を実施しておく
    stepInitializer = make_unique<StepInitializer>();
    stepInitializer->Initialize();

	std::function<void()> stepFunc = [this]() {
		// 軽量な初期化 (必須 UI とカメラ)
		camera = make_unique<Camera>();
		camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
		camera->SetTranslate({ 0.0f, 2.8f, -4.4f });

		SkyBox::GetInstance()->SetCamera(camera.get());

		input = Input::GetInstance();
		input->ShowMouseCursor(true);

		Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

		ParticleManager::GetInstance()->SetCamera(camera.get());
		};
    stepInitializer->AddStep(stepFunc);

	stepFunc = [this]() {
		// プレースホルダとなるオブジェクトは最低限生成する
		title = make_unique<Object3d>();
		title->Initialize();
		// モデルの本読み込みは後のステップで行う
		title->SetTranslate({ -0.04f, 2.0f, 0.0f });
		title->SetRotate(Vector3(SwapRadian(10.0f), 0.01f, 0.0f));

		playerModel = make_unique<Object3d>();
		playerModel->Initialize();
		playerModel->SetTranslate({ 0.0f, 0.1f, 0.0f });

		stageModel = make_unique<Object3d>();
		stageModel->Initialize();

		// UIの初期化は軽めなのでここで実行
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
		gamePad->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

		gamePadOnFrame = make_unique<Sprite>();
		gamePadOnFrame->Initialize("Resources/Sprite/UI/gamepadONFrame.png");
		gamePadOnFrame->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

		credit_sound = make_unique<Sprite>();
		credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
		credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
		credit_sound->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) });

		TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");
		SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

		};
    stepInitializer->AddStep(stepFunc);

	stepFunc = [this]() {
		// モデル本体の読み込み (比較的重い)
		// ModelManager は内部で同期読み込みだが、分割して呼ぶことでフレームブロックを分散
		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/title", "title.gltf", true);
		title->SetModel("Resources/Model/gltf/title", "title.gltf", true);

		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/char", "idle.gltf", true, true);
		playerModel->SetModel("Resources/Model/gltf/char", "idle.gltf", true, true);
		playerModel->ToggleStartAnimation();

		ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/Stage/map01", "map01.gltf", true);
		stageModel->SetModel("Resources/Model/gltf/Stage/map01", "map01.gltf", true);
		};
    stepInitializer->AddStep(stepFunc);

	stepFunc = [this]() {
		// オーディオなど残りの読み込み
		Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select", 1.0f);
		Audio::GetInstance()->LoadMP3("Resources/sound/enter.mp3", "enter", 1.0f);
		Audio::GetInstance()->LoadMP3("Resources/sound/Experimenta_Model_short.mp3", "bgm", 0.2f);

		Audio::GetInstance()->Play("bgm", true);
		Audio::GetInstance()->SetMasterVolume(0.0f);
		};
    stepInitializer->AddStep(stepFunc);

	//InitializeStep();
}

void TitleScene::Update() {

	// 初期化が完了していなければ一フレームごとに初期化ステップを進行
	if (!assetsLoaded) {
		stepInitializer->Update();

		if (stepInitializer->IsFinished())
		{
			assetsLoaded = true;
		}
		// 描画側では読み込み中UIを表示するため、Updateの残りは最小限にする
		// それでも一部必要なUpdateは行う
		input = Input::GetInstance();
		input->ShowMouseCursor(true);

		uiFrame->Update();
		gamePad->Update();
		gamePadOnFrame->Update();
		credit_sound->Update();
		camera->Update();
		SkyBox::GetInstance()->Update();
		return;
	}

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

	// 読み込み中ならローディング画面を優先して描画
	if (!assetsLoaded)
	{
		// 最低限の背景(ステージとタイトルプレースホルダ)を描画し、UIで読み込み中を示す
		SpriteBase::GetInstance()->ShaderDraw();

		Object3dBase::GetInstance()->ShaderDraw();

		stageModel->Draw();
		title->Draw();

		SkinningObject3dBase::GetInstance()->ShaderDraw();

		playerModel->Draw();

		WireFrameObjectBase::GetInstance()->ShaderDraw();

		ParticleManager::GetInstance()->Draw();

		SpriteBase::GetInstance()->ShaderDraw();

		// 読み込み中メッセージ
		startUI->Draw();
		
		// loading フレーム (uiFrame を仮利用)
		uiFrame->Draw();
		
		gamePad->Draw();
		// 進捗表示を追加したければここに描画処理を入れる
		return;
	}

	// assetsLoaded == true の通常描画
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