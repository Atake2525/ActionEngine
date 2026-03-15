#include "GameScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "GameTime.h"
#include "JsonLoader.h"
#include "StageCount.h"
#include "TutorialStage.h"
#include "EasingUtility.h"
#include "Light.h"

using namespace std;
using namespace ActionEngine::Stage;

void GameScene::Initialize() {

    TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

    camera = make_unique<Camera>();
    camera->SetTranslate({ 0.0f, 1.8f, 0.0f });
    camera->SetFarClipDistance(0.0f);

    SkyBox::GetInstance()->SetCamera(camera.get());
    SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");
    SkyBox::GetInstance()->SetSunPoewr(0.0f);

    input = Input::GetInstance();

    Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

    m_pPlayer = make_unique<Player>();
    stage = make_unique<TutorialStage>();
    stage->Initialize(m_pPlayer.get(), camera.get());
    m_pPlayer->Initialize(camera.get(), stage->GetJsonName());
    m_pPlayer->Update();

    Light::GetInstance()->SetRadius(0.1f);
    GameTime::GetInstance()->SetDeltaPoint();
    FadeManager::GetInstance()->FadeIn(1.0f);
    m_pause = make_unique<Pause>();
    m_pause->Initialize();

    m_scenePhase = ScenePhase::FadeIn;

}

void GameScene::Update() {

    // ポーズはどのフェーズでも行えるようにする
    m_pause->Update();
    if (m_pause->IsPause())
    {
        return;
    }

    float farClipDist = 0.0f;
    float radius = 0.0f;
    // フェーズ管理
    switch (m_scenePhase)
    {
    case ScenePhase::FadeIn: // シーン遷移演出フェーズ(入)
        if (FadeManager::GetInstance()->CompleteFade() || !FadeManager::GetInstance()->IsFade())
        {
            m_scenePhase = ScenePhase::Ready;
            m_finalScanRadius = m_finalFarClipDistance + Light::GetInstance()->GetScanWidth() * 3.0f;
        }
        break;
    case ScenePhase::Ready: // スタート演出フェーズ
        m_startTimer += GameTime::GetInstance()->GetDeltaTime() / m_startTime;
        m_startTimer = clamp(m_startTimer, 0.0f, 1.0f);

        
        switch (m_readyNumber)
        {
        case 0:
            farClipDist = EaseOutExpo(m_startTimer, 0.0f, m_finalFarClipDistance);
            camera->SetFarClipDistance(farClipDist);
            break;
        case 1:
            radius = EaseOutExpo(m_startTimer, 0.0f, m_finalScanRadius);
            Light::GetInstance()->SetRadius(radius);

            SkyBox::GetInstance()->SetSunPoewr(m_startTimer);
            
            break;
        }

        if (m_readyNumber == 1 && m_startTimer == 1.0f)
        {
            m_scenePhase = ScenePhase::Game;
            m_readyNumber = 0;
        }

        if (m_startTimer == 1.0f)
        {
            m_readyNumber++;
            m_startTimer = 0.0f;
        }
        break;
    case ScenePhase::Game: // プレイフェーズ
        m_pPlayer->Update();
        break;
    //case ScenePhase::FadeOut: // シーン遷移演出フェーズ(出)
    //    break;
    }


    stage->Update();

#ifndef NDEBUG
    if (input->TriggerKey(DIK_ESCAPE))
    {
        finished = true;
    }

    if (input->TriggerKey(DIK_F11))
    {
        cursorshow = !cursorshow;
    }
#else

#endif // !NDEBUG

    
    input->ShowMouseCursor(cursorshow);

    if (input->TriggerKey(DIK_1))
    {
        Audio::GetInstance()->Play2D("bgm", { 0.0f, 0.0f }, false);
    }


    SkyBox::GetInstance()->Update();


    if (input->TriggerKey(DIK_R))
    {
        SceneManager::GetInstance()->SetNextScene("GAMESCENE");
    }

   
    camera->Update();

}

void GameScene::Draw() {

    SpriteBase::GetInstance()->ShaderDraw();


    Object3dBase::GetInstance()->ShaderDraw();

    stage->DrawObject3d();

    SkinningObject3dBase::GetInstance()->ShaderDraw();


    SpriteBase::GetInstance()->ShaderDraw();

    m_pause->Draw();
}

void GameScene::Finalize() {
    stage->Finalize();
}