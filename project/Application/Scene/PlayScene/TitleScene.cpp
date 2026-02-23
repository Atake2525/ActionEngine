#include "TitleScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "WinApp.h"
#include "GameTime.h"
#include "EasingUtility.h"
#include "Light.h"

using namespace std;

void TitleScene::Initialize() {

    //ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

    camera = make_unique<Camera>();
    camera->SetRotate(Vector3(SwapRadian(-12.0f), SwapRadian(1.5f), 0.0f));
    camera->SetTranslate({ -1.0f, 0.0f, -3.0f });

    TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

    SkyBox::GetInstance()->SetCamera(camera.get());
    SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

    input = Input::GetInstance();
    input->ShowMouseCursor(true);

    Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

    ParticleManager::GetInstance()->SetCamera(camera.get());

    charModel = make_unique<Object3d>();
    charModel->Initialize();
    charModel->SetModel("Resources/Model/gltf", "TitleSceneChar.gltf", true, true);
    charModel->ToggleStartAnimation();
    charModel->SetRotate({ 0.0f, SwapRadian(180.0f), 0.0f});

    bootScreen = make_unique<Object3d>();
    bootScreen->Initialize();
    bootScreen->SetModel("Resources/Model/obj/Title", "TitleScene_01.obj", true);

    gamePad = make_unique<Sprite>();
    gamePad->Initialize("Resources/Sprite/UI/gamepad.png");
    //gamePad->SetAnchorPoint({ 0.5f, 0.5f });
    gamePad->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() - gamePad->GetTextureSize().x - 10.0f), float(WinApp::GetInstance()->GetkClientHeight() - gamePad->GetTextureSize().y - 10.0f) });

    credit_sound = make_unique<Sprite>();
    credit_sound->Initialize("Resources/Sprite/UI/credit_sound.png");
    credit_sound->SetAnchorPoint({ 0.5f, 0.5f });
    credit_sound->SetPosition({ float(WinApp::GetInstance()->GetkClientWidth() / 2.0f), float(WinApp::GetInstance()->GetkClientHeight() / 2.0f) });

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
}

void TitleScene::Update() {

    selectPre = select;

    if (!start_)
    {
        if (FadeManager::GetInstance()->CompleteFade())
        {
            start_ = true;
        }
        else
        {
            return;
        }
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

    gamePad->Update();

    credit_sound->Update();

    bootScreen->Update();

    charModel->Update();

    camera->Update();

    SkyBox::GetInstance()->Update();

    //input->Update();




}

void TitleScene::Draw() {

    SpriteBase::GetInstance()->ShaderDraw();



    Object3dBase::GetInstance()->ShaderDraw();

    bootScreen->Draw();
    //title->Draw();

    SkinningObject3dBase::GetInstance()->ShaderDraw();

    charModel->Draw();

    WireFrameObjectBase::GetInstance()->ShaderDraw();


    ParticleManager::GetInstance()->Draw();

    SpriteBase::GetInstance()->ShaderDraw();

    SpriteBase::GetInstance()->ShaderDraw();
    //uiFrame->Draw();
    //SceneFadeManager::GetInstance()->Draw();

}

void TitleScene::Finalize() {

}