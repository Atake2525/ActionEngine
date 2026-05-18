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

using namespace std;

void TitleScene::Initialize() {

    m_pCamera = make_unique<Camera>();
    m_pCamera->SetRotate(Vector3(SwapRadian(11.5f), SwapRadian(1.5f), 0.0f));
    m_pCamera->SetTranslate({ -1.0f, 1.6f, -3.4f });
    m_screenChangeTransformPre = m_pCamera->GetTransform();
    m_pCamera->Update();

    TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

    SkyBox::GetInstance()->SetCamera(m_pCamera.get());
    SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");
    SkyBox::GetInstance()->SetSunPoewr(1.0f);

    m_pInput = Input::GetInstance();
    m_mouseCursor = std::make_unique<MouseCursor>();
    m_mouseCursor->Initialize("Resources/Sprite/Cursor_Hover.png", "Resources/Sprite/Cursor_Press.png");
    m_mouseCursor->SetCursorPosition(WinApp::GetInstance()->GetWindowSize() / 2.0f);

    Object3dBase::GetInstance()->SetDefaultCamera(m_pCamera.get());

    ParticleManager::GetInstance()->SetCamera(m_pCamera.get());

    m_charModel = make_unique<Object3d>();
    m_charModel->Initialize();
    m_charModel->SetModel("Resources/Model/gltf", "TitleSceneChar.gltf", true, true);
    m_charModel->AddAnimation("Resources/Model/gltf", "sceneChange_Animation.gltf", "TitleScreen");
    m_charModel->ToggleStartAnimation();
    m_charModel->SetRotate({ 0.0f, SwapRadian(180.0f), 0.0f});
    m_charModel->Update();

    m_bootScreen = make_unique<Object3d>();
    m_bootScreen->Initialize();
    m_bootScreen->SetModel("Resources/Model/obj/Title", "TitleScene_01.obj", true);
    m_bootScreen->Update();

    Vector2 windowSize = { WinApp::GetInstance()->GetWindowSize() };
    
    m_titleSceneUI = make_unique<TitleSceneUI>(m_mouseCursor.get());

    m_gamePad = make_unique<Sprite>();
    m_gamePad->Initialize("Resources/Sprite/UI/gamepad.png");
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

    m_credit_sound = make_unique<Sprite>();
    m_credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
    m_credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
    m_credit_sound->SetPosition({ windowSize.x / 4.0f, windowSize.y / 2.0f });

    m_credit = make_unique<Sprite>();
    m_credit->Initialize("Resources/Sprite/UI/credit.png");
    m_credit->SetAnchorPoint(ANCHORPOINT_LEFTBOTTOM);
    m_credit->SetPosition({ 30.0f, windowSize.y - 30.0f });

    FadeManager::GetInstance()->FadeIn(1.0f);

    Light::GetInstance()->SetPositionPointLight({ 0.2f, 1.9f, 3.4f });
    Light::GetInstance()->SetIntensityPointLight(1.0f);
    Light::GetInstance()->SetRadiusPointLight(4.0f);
    Light::GetInstance()->SetColorPointLight(Vector4{ 1.0f, 93.0f / 255.0f, 0.0f, 1.0f });

    Light::GetInstance()->SetDirectionDirectionalLight({ 0.174f, -0.35f, 1.0f });
    Light::GetInstance()->SetIntensityDirectionalLight(0.5f);
    Light::GetInstance()->SetRadius(m_pCamera->GetFarClipDistance());

    m_sceneScreen = TitleSceneScreen::BootScreen;

}

void TitleScene::Update() {

    if (FadeManager::GetInstance()->IsFade())
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
            m_screenChangeTimer += GameTime::GetInstance()->GetDeltaTime() / m_screenChangeTime[m_changeNum];
            m_screenChangeTimer = std::clamp(m_screenChangeTimer, 0.0f, 1.0f);
            Transform cameraT = Transform::Default;
            cameraT.rotate = Lerp(m_screenChangeTransformPre.rotate, m_screenChangeTransform[m_changeNum].rotate, m_screenChangeTimer);
            cameraT.translate = Lerp(m_screenChangeTransformPre.translate, m_screenChangeTransform[m_changeNum].translate, m_screenChangeTimer);
            m_pCamera->SetTransform(cameraT);

            if (m_screenChangeTimer == 1.0f && m_changeNum == 0)
            {
                m_screenChangeTimer = 0.0f;
                m_screenChangeTransformPre = cameraT;
                m_changeNum++;
                FadeManager::GetInstance()->FadeOut(0.4f);
                FadeManager::GetInstance()->SetColor({ 1.0f, 1.0f, 1.0f });
            }

            if (m_screenChangeTimer == 1.0f && m_changeNum == 1)
            {
                SceneManager::GetInstance()->SetNextScene("GAMESCENE");
                m_screenChange = false;
            }
        }

        // creditの表示
        //if (CollisionUISprite(m_credit->GetAABB(), m_mouseCursor->GetCursorPos()))
        //{
        //    if (m_credit->GetColor().y != 0.0f)
        //    {
        //        Audio::GetInstance()->Play("select");   
        //    }
        //    if (m_pInput->TriggerMouse(0))
        //    {
        //        m_showCredit = !m_showCredit;
        //        Audio::GetInstance()->Play("select_enter");
        //    }
        //    m_credit->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        //}
        //else
        //{
        //    m_credit->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        //}

        //// ゲームパッドを読み込みなおす
        //if (CollisionUISprite(m_gamePad->GetAABB(), m_mouseCursor->GetCursorPos()) && m_pInput->TriggerMouse(0))
        //{
        //    m_pInput->UpdateDevice();
        //    if (m_pInput->IsConnectedController())
        //    {
        //        m_gamePad->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        //    }
        //}

        m_gamePad->Update();
        m_credit->Update();
        m_credit_sound->Update();

        break;
    }

    Vector2 mousePos = m_pInput->GetMousePos2();
    ImGui::Begin("MousePos");
    ImGui::DragFloat2("MousePos", &mousePos.x);
    ImGui::End();

    m_bootScreen->Update();

    m_charModel->Update();

    SkyBox::GetInstance()->Update();

    m_pCamera->Update();
    m_mouseCursor->Update();
}

void TitleScene::Draw() {

    switch (m_sceneScreen)
    {
    case TitleSceneScreen::BootScreen:

        Object3dBase::GetInstance()->ShaderDraw();

        m_bootScreen->Draw();
        m_charModel->Draw();

        Render2DBase::GetInstance()->ShaderDraw();

        m_titleSceneUI->DrawBootScreen();

        break;
    case TitleSceneScreen::TitleScreen:

        Object3dBase::GetInstance()->ShaderDraw();

        m_bootScreen->Draw();
        m_charModel->Draw();

        SkinningObject3dBase::GetInstance()->ShaderDraw();

        Render2DBase::GetInstance()->ShaderDraw();

        Render2DBase::GetInstance()->ShaderDraw();

        m_gamePad->Draw();
        m_credit->Draw();
        m_titleSceneUI->DrawTitleScreen();
        if (m_showCredit)
        {
            m_credit_sound->Draw();
        }

        break;
    }

    Render2DBase::GetInstance()->ShaderDraw();
    m_mouseCursor->Draw();
}

void TitleScene::Finalize() {

}
