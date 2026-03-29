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

    //ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

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
    m_pInput->ShowMouseCursor(false);
    m_mouseCursor = std::make_unique<MouseCursor>();
    m_mouseCursor->Initialize("Resources/Sprite/Cursor_Hover.png", "Resources/Sprite/Cursor_Press.png");

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

    Vector2 windowSize = { float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()) };
    m_startUi = make_unique<Sprite>();
    m_startUi->Initialize("Resources/Sprite/UI/ui_start.png");
    m_startUi->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    Vector2 size = m_startUi->GetTextureSize();
    m_startUi->SetScale(size * 0.2f);
    m_startUi->SetAnchorPoint({ 0.5f, 0.5f });
    m_startUi->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f });
    m_startUi->Update();


    m_exitUi = make_unique<Sprite>();
    m_exitUi->Initialize("Resources/Sprite/UI/ui_exit.png");
    m_exitUi->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    size = m_exitUi->GetTextureSize();
    m_exitUi->SetScale(size * 0.2f);
    m_exitUi->SetAnchorPoint({ 0.5f, 0.5f });
    m_exitUi->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f + (size.y * 0.3f) });
    m_exitUi->Update();

    m_pressAnyKey = make_unique<Sprite>();
    m_pressAnyKey->Initialize("Resources/Sprite/UI/press_any_key.png");
    size = m_pressAnyKey->GetTextureSize();
    m_pressAnyKey->SetScale({ size.x * 0.3f, size.y * 0.3f });
    m_pressAnyKey->SetAnchorPoint({ 0.5f, 1.0f });
    m_pressAnyKey->SetPosition({ windowSize.x * 0.6f, windowSize.y * 1.014f });
    m_pressAnyKey->SetRotatioin(-SwapRadian(1.0f));
    m_pressAnyKey->Update();

    m_gamePad = make_unique<Sprite>();
    m_gamePad->Initialize("Resources/Sprite/UI/gamepad.png");
    //m_gamePad->SetAnchorPoint({ 0.5f, 0.5f });
    m_gamePad->SetPosition({ windowSize.x - m_gamePad->GetTextureSize().x - 10.0f, windowSize.y - m_gamePad->GetTextureSize().y - 10.0f });

    m_credit_sound = make_unique<Sprite>();
    m_credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
    m_credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
    m_credit_sound->SetPosition({ windowSize.x / 2.0f, windowSize.y / 2.0f });

    Audio::GetInstance()->LoadMP3("Resources/sound/select.mp3", "select", 1.0f);
    Audio::GetInstance()->LoadMP3("Resources/sound/enter.mp3", "enter", 1.0f);
    //Audio::GetInstance()->LoadMP3("Resources/sound/Experimenta_Model_short.mp3", "bgm", 0.2f);

    //Audio::GetInstance()->Play("bgm", true);

    //Audio::GetInstance()->SetMasterVolume(0.0f);
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

    switch (m_sceneScreen)
    {
    case TitleScene::TitleSceneScreen::BootScreen:

        


        if (m_pInput->PressAnyKey() || m_pInput->PressAnyButton() || m_pInput->TriggerMouse(0) || m_pInput->TriggerMouse(1))
        {
            m_sceneScreen = TitleSceneScreen::TitleScreen;

            m_startUi->Update();
            m_exitUi->Update();

        }
        // 何かしらキーを押したらBootScreenからTitleScreenに切り替える
        m_pressAnyKey->Update();

        break;
    case TitleScene::TitleSceneScreen::TitleScreen:
        Vector2 cursorPosition = m_mouseCursor->GetCursorPos();
        Vector3 pos = { cursorPosition.x, cursorPosition.y, 0.0f };
        AABB aabb = { {pos},{pos} };
        // UIにマウスカーソルが入っている時、クリックしたときの処理
        if (CollisionSprite(m_startUi->GetAABB(), aabb))
        {
            m_startUi->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

            if (m_pInput->TriggerMouse(0))
            {
                m_screenChange = true;
                m_charModel->ChangePlayAnimation("TitleScreen");
                m_charModel->ResetAnimationTime();
            }
        }
        else
        {
            m_startUi->SetColor({ 0.0f, 0.0f, 0.0f,1.0f });
        }

        if (CollisionSprite(m_exitUi->GetAABB(), aabb))
        {
            m_exitUi->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

            if (m_pInput->TriggerMouse(0))
            {
                finished = true;
            }
        }
        else
        {
            m_exitUi->SetColor({ 0.0f, 0.0f, 0.0f,1.0f });
        }

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
                /*camera->SetTranslate({ 0.0f, 0.2f, -4.5f });
                camera->SetRotate({ SwapRadian(-9.5f), 0.0f, 0.0f });
                m_screenChangeTimer = 0.0f;*/
            }
        }
       

        m_startUi->Update();
        m_exitUi->Update();

        break;
    }

    //camera->SetParent(m_charModel->GetJointMatrix("Head"));

    m_bootScreen->Update();

    m_charModel->Update();

    SkyBox::GetInstance()->Update();

    m_pCamera->Update();
    m_mouseCursor->Update();
}

void TitleScene::Draw() {

    switch (m_sceneScreen)
    {
    case TitleScene::TitleSceneScreen::BootScreen:

        Object3dBase::GetInstance()->ShaderDraw();

        m_bootScreen->Draw();

        SkinningObject3dBase::GetInstance()->ShaderDraw();

        m_charModel->Draw();

        SpriteBase::GetInstance()->ShaderDraw();

        if (!m_screenChange)
        {
            m_pressAnyKey->Draw();
        }

        break;
    case TitleScene::TitleSceneScreen::TitleScreen:

        Object3dBase::GetInstance()->ShaderDraw();

        m_bootScreen->Draw();

        SkinningObject3dBase::GetInstance()->ShaderDraw();

        m_charModel->Draw();

        SpriteBase::GetInstance()->ShaderDraw();

        SpriteBase::GetInstance()->ShaderDraw();

        m_startUi->Draw();
        m_exitUi->Draw();

        break;
    }

    SpriteBase::GetInstance()->ShaderDraw();
    m_mouseCursor->Draw();

}

void TitleScene::Finalize() {

}
