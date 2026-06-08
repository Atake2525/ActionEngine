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

    TextureManager::GetInstance()->LoadTexture("Resources/white1x1.dds");

    m_pCamera = make_unique<Camera>();
    m_pCamera->SetTranslate({ 0.0f, 1.8f, 0.0f });
    m_pCamera->SetFarClipDistance(0.0f);

    SkyBox::GetInstance()->SetCamera(m_pCamera.get());
    SkyBox::GetInstance()->SetTexture("Resources/white1x1.dds");
    SkyBox::GetInstance()->SetSunPoewr(0.0f);

    m_pInput = Input::GetInstance();
    m_mouseCursor = std::make_unique<MouseCursor>();
    m_mouseCursor->Initialize("Resources/Sprite/Cursor_Hover.png", "Resources/Sprite/Cursor_Press.png");
    m_mouseCursor->SetShowCursor(false);

    Object3dBase::GetInstance()->SetDefaultCamera(m_pCamera.get());

    m_pPlayer = make_unique<Player>();
    m_pStage = make_unique<TutorialStage>();
    m_pStage->Initialize(m_pPlayer.get(), m_pCamera.get());
    m_pPlayer->Initialize(m_pCamera.get(), m_pStage->GetJsonName());
    m_pCamera->Update();
    m_pPlayer->UpdateModel();

    m_pPlayerUI = make_unique<PlayerUI>();
    m_pPlayerUI->Initialize(m_pPlayer.get());

    Light::GetInstance()->SetRadius(0.1f);
    GameTime::GetInstance()->SetDeltaPoint();
    FadeManager::GetInstance()->FadeIn(1.0f);
    m_pPause = make_unique<Pause>();
    m_pPause->Initialize();

    m_scenePhase = ScenePhase::FadeIn;
    m_pInput->ShowMouseCursor(m_cursorShow);
}

void GameScene::Update() {

    // ポーズはどのフェーズでも行えるようにする
    m_pPause->Update();
    // ポーズ時にマウスカーソルを使うため前の方で更新
    m_mouseCursor->Update();
    if (m_pPause->IsPause())
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
            farClipDist = EaseOutExpo(0.0f, m_finalFarClipDistance, m_startTimer);
            m_pCamera->SetFarClipDistance(farClipDist);
            break;
        case 1:
            radius = EaseOutExpo(0.0f, m_finalScanRadius, m_startTimer);
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
        m_pPlayerUI->Update();
        break;
    }

    m_pCamera->Update();
    m_pPlayer->UpdateModel();

    m_pStage->Update();

#ifndef NDEBUG
    if (m_pInput->TriggerKey(DIK_F11))
    {
        m_cursorShow = !m_cursorShow;
        m_pInput->ShowMouseCursor(m_cursorShow);
    }
#else

#endif // !NDEBUG



    SkyBox::GetInstance()->Update();
}

void GameScene::Draw() {

    Render2DBase::GetInstance()->ShaderDraw();


    Object3dBase::GetInstance()->ShaderDraw();

    m_pStage->DrawObject3d();
    m_pPlayer->Draw();

    Render2DBase::GetInstance()->ShaderDraw();

    m_pStage->DrawBackSprite();
    m_pPlayerUI->Draw();
    m_pPause->Draw();
    m_mouseCursor->Draw();
}

void GameScene::Finalize() {
    m_pStage->Finalize();
}
