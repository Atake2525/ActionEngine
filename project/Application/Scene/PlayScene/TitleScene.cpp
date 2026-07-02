#include "TitleScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "WinApp.h"
#include "GameTime.h"
#include "EasingUtility.h"
#include "Light.h"
#include "FadeManager.h"
#include "Collision.h"
#include "Light.h"
#include "EngineContext.h"
#include "RenderObjectFactory.h"

using namespace std;

void TitleScene::Initialize() {
    AppContext& context = *m_pContext;

    m_pCamera = make_unique<Camera>(context.engine.platform.window);
    m_pCamera->SetRotate(Vector3(SwapRadian(11.5f), SwapRadian(1.5f), 0.0f));
    m_pCamera->SetPosition({ -1.0f, 1.6f, -3.4f });
    m_screenChangeTransformPre = m_pCamera->GetTransform();
    m_pCamera->Update();

    context.engine.assets.textures.LoadTexture("Resources/white1x1.dds");

    context.world.skyBox.SetCamera(m_pCamera.get());
    context.world.skyBox.SetTexture("Resources/white1x1.dds");
    context.world.light.SetIntensityDirectionalLight(1.0f);

    m_pInput = &context.engine.platform.input;

    context.engine.graphics.object3DBase.SetDefaultCamera(m_pCamera.get());

    context.world.particles.SetCamera(m_pCamera.get());

    m_charModel = m_renderList.AddObject3d(context.game.object3dFactory.Create());
    Model* charModel = context.engine.assets.models.LoadModel("Resources/Model/gltf", "TitleSceneChar.gltf", true);
    m_charModel->SetModel(charModel);
    m_charModel->AddAnimation("Resources/Model/gltf", "sceneChange_Animation.gltf", "TitleScreen");
    m_charModel->ToggleStartAnimation();
    m_charModel->SetRotate({ 0.0f, SwapRadian(180.0f), 0.0f});
    m_charModel->SetEnableLighting(true);
    m_charModel->SetEnvironmentCoefficient(0.5f);
    m_charModel->Update();

    m_bootScreen = m_renderList.AddObject3d(context.game.object3dFactory.Create());
    Model* bootScreenModel = context.engine.assets.models.LoadModel("Resources/Model/obj/Title", "TitleScene_01.obj");
    m_bootScreen->SetModel(bootScreenModel);
    m_bootScreen->Update();
    m_bootScreen->SetEnableLighting(true);

    Vector2 windowSize = { context.engine.platform.window.GetWindowSize() };
    
    m_gamePad = m_renderList.AddSprite(context.game.spriteFactory.Create("Resources/Sprite/UI/gamepad.png"));
    m_gamePad->SetPosition({ windowSize.x - m_gamePad->GetTextureSize().x - 10.0f, windowSize.y - m_gamePad->GetTextureSize().y - 10.0f });
    // ゲームパッドが接続されている場合は、ゲームパッドのアイコンをAlpha1.0fで表示する
    if (m_pInput->IsConnectedController())
    {
        m_gamePad->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    }
    else
    {
        m_gamePad->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });
    }

    m_credit_sound = context.game.spriteFactory.Create("Resources/Sprite/UI/credit_sound.png");
    m_credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
    m_credit_sound->SetPosition({ windowSize.x / 4.0f, windowSize.y / 2.0f });

    m_creditUI = make_unique<UI::Button>();
    m_creditUI->SetContext(context.game.spriteFactory);
    m_creditUI->Initialize("Resources/Sprite/UI/credit.png", *m_pInput);
    m_creditUI->SetPosition({ windowSize.x * 0.05f, windowSize.y * 0.95f });
    std::function<void()>creditFunc = [this]() {
        m_showCredit = !m_showCredit;
        m_pContext->engine.assets.audio.Play("select");
        };
    m_creditUI->SetActiveReaction(creditFunc);
    m_creditUI->AddInteractBinding(UI::InputTrigger{ .key = DIK_SPACE, .mouseButton = 0, .controller = Controller::A });
    m_creditUI->AddInteractBinding(UI::InputTrigger{ .key = DIK_RETURN });
    m_creditUI->ShowThisFrame();

    m_titleSceneUI = make_unique<TitleSceneUI>();
    m_titleSceneUI->Initialize(context.game.spriteFactory, context.engine.platform.input, context.engine.platform.window, context.engine.assets.audio);

    context.engine.presentation.fade.FadeIn(1.0f);

	camera = new Camera();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

    context.world.light.SetDirectionDirectionalLight({ 0.174f, -0.35f, 1.0f });
    context.world.light.SetIntensityDirectionalLight(1.0f);
    context.world.light.SetRadius(m_pCamera->GetFarClipDistance());

    m_sceneScreen = TitleSceneScreen::BootScreen;

}

void TitleScene::Update() {
    AppContext& context = *m_pContext;

    if (context.engine.presentation.fade.IsFade())
    {
        return;
    }

    if (m_pInput->TriggerKey(DIK_ESCAPE))
    {
        finished = true;
    }

    m_titleSceneUI->Update(m_sceneScreen);

    std::string pressUI = m_titleSceneUI->GetPressUI();

    if (pressUI == "bootScreen")
    {
        m_sceneScreen = TitleSceneScreen::TitleScreen;
    }
    else if (pressUI == "start" && !m_showCredit)
    {
        m_screenChange = true;
        m_charModel->ChangePlayAnimation("TitleScreen");
        m_charModel->ResetAnimationTime();
    }
    else if (pressUI == "exit" && !m_showCredit)
    {
        finished = true;
    }

    switch (m_sceneScreen)
    {
    case TitleSceneScreen::TitleScreen:
        
        if (m_screenChange)
        {
            m_screenChangeTimer += context.engine.platform.time.GetDeltaTime() / m_screenChangeTime[m_changeNum];
            m_screenChangeTimer = std::clamp(m_screenChangeTimer, 0.0f, 1.0f);
            Transform cameraT = Transform::Default;
            cameraT.rotate = Lerp(m_screenChangeTransformPre.rotate, m_screenChangeTransform[m_changeNum].rotate, m_screenChangeTimer);
            cameraT.position = Lerp(m_screenChangeTransformPre.position, m_screenChangeTransform[m_changeNum].position, m_screenChangeTimer);
            m_pCamera->SetTransform(cameraT);

            if (m_screenChangeTimer == 1.0f && m_changeNum == 0)
            {
                m_screenChangeTimer = 0.0f;
                m_screenChangeTransformPre = cameraT;
                m_changeNum++;
                context.engine.presentation.fade.FadeOut(0.4f);
                context.engine.presentation.fade.SetColor({ 1.0f, 1.0f, 1.0f });
            }

            if (m_screenChangeTimer == 1.0f && m_changeNum == 1)
            {
                m_sceneManager->SetNextScene("GAMESCENE");
                m_screenChange = false;
            }
        }

        // creditの表示
        m_creditUI->Update();

        m_credit_sound->Update();

        break;
    }

    Vector2 mousePos = m_pInput->GetMousePos2();

    m_renderList.Update();

    context.world.skyBox.Update(m_pContext->world.light);

    m_pCamera->Update();
}

void TitleScene::Draw() {
    AppContext& context = *m_pContext;

    switch (m_sceneScreen)
    {
    case TitleSceneScreen::BootScreen:

        context.engine.graphics.object3DBase.ShaderDraw();

        m_renderList.DrawObject3d();

        context.engine.graphics.render2DBase.ShaderDraw();

        m_titleSceneUI->DrawBootScreen();

        break;
    case TitleSceneScreen::TitleScreen:

        context.engine.graphics.object3DBase.ShaderDraw();

        m_renderList.DrawObject3d();

        context.engine.graphics.render2DBase.ShaderDraw();

        m_renderList.DrawSprites();
        m_creditUI->Draw();
        m_titleSceneUI->DrawTitleScreen();
        if (m_showCredit)
        {
            m_credit_sound->Draw();
        }

        break;
    }

    context.engine.graphics.render2DBase.ShaderDraw();
}

void TitleScene::Finalize() {

}
