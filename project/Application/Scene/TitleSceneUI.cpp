#include "TitleSceneUI.h"
#include "Collision.h"
#include "Audio.h"

TitleSceneUI::TitleSceneUI(MouseCursor* mouseCursor) {
    // STARTとEXITのUIスプライトの初期化
    
    m_pressAnyKeySprite = std::make_unique<Sprite>();
    m_pressAnyKeySprite->Initialize("Resources/Sprite/UI/press_any_key.png");
    Vector2 size = m_pressAnyKeySprite->GetTextureSize();
    m_pressAnyKeySprite->SetScale({ size.x * 0.3f, size.y * 0.3f });
    m_pressAnyKeySprite->SetAnchorPoint({ 0.5f, 1.0f });
    Vector2 windowSize = { WinApp::GetInstance()->GetWindowSize() };
    m_pressAnyKeySprite->SetPosition({ windowSize.x * 0.6f, windowSize.y * 1.014f });
    m_pressAnyKeySprite->SetRotatioin(-SwapRadian(1.0f));
    m_pressAnyKeySprite->Update();

    m_pInput = Input::GetInstance();
    m_mouseCursor = mouseCursor;

    m_startButton = std::make_unique<UI::Button>();
    m_startButton->Initialize("Resources/Sprite/UI/ui_start.png", *m_pInput);
    m_startButton->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f });
    // 音を鳴らす
    std::function<void()> startButtonReaction = [this]() {
        Audio::GetInstance()->Play("select_enter");
        };
    UI::InteractionReaction reactions = { .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f}, .custom = startButtonReaction };
    m_startButton->SetOnSelectedReaction(reactions);
    m_startButton->SetOnPressedReaction(reactions);
    m_startButton->SetOnSubmittedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor = {1.0f, 1.0f, 0.0f, 1.0f}, .scale = false, .custom = startButtonReaction });
    startButtonReaction = [this]() {
        m_pressUI = "start";
        Audio::GetInstance()->Play("select_enter");
        };
    m_startButton->SetActiveReaction(startButtonReaction);
    m_startButton->ShowThisFrame();

    m_exitButton = std::make_unique<UI::Button>();
    m_exitButton->Initialize("Resources/Sprite/UI/ui_exit.png", *m_pInput);
    Vector2 exitButtonSize = m_exitButton->GetScale();
    m_exitButton->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f + exitButtonSize.y * 1.2f });
    // 音を鳴らす
    std::function<void()> exitButtonReaction = [this]() {
        Audio::GetInstance()->Play("select_enter");
        };
    reactions = { .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f}, .custom = exitButtonReaction };
    m_exitButton->SetOnSelectedReaction(reactions);
    m_exitButton->SetOnPressedReaction(reactions);
    exitButtonReaction = [this]() {
        m_pressUI = "exit";
        Audio::GetInstance()->Play("select_enter");
        };
    m_exitButton->SetActiveReaction(exitButtonReaction);
    m_exitButton->ShowThisFrame();

}

TitleSceneUI::~TitleSceneUI()
{}

void TitleSceneUI::Update(const TitleSceneScreen& screen) {

    switch (screen)
    {
    case TitleSceneScreen::BootScreen:
        if (m_pInput->PressAnyKey() || m_pInput->PressAnyButton() || m_pInput->TriggerMouse(0) || m_pInput->TriggerMouse(1))
        {
            m_pressUI = "bootScreen";
            Audio::GetInstance()->Play("select_enter");
        }
        break;

    case TitleSceneScreen::TitleScreen:

        m_startButton->Update();
        m_exitButton->Update();

        break;
    }
}

void TitleSceneUI::DrawBootScreen() {
    m_pressAnyKeySprite->Draw();
}

void TitleSceneUI::DrawTitleScreen() {
    m_startButton->Draw();
    m_exitButton->Draw();
}
