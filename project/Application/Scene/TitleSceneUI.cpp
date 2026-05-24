#include "TitleSceneUI.h"
#include "Collision.h"
#include "Audio.h"
#include "UIElement.h"
#include "UIButton.h"
#include "SelectionGroup.h"

TitleSceneUI::TitleSceneUI() {
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

    std::unique_ptr<UI::Element> m_startButton;
    m_startButton = std::make_unique<UI::Button>();
    m_startButton->Initialize("Resources/Sprite/UI/ui_start.png", *m_pInput);
    m_startButton->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f });
    // 音を鳴らす
    std::function<void()> startButtonReaction = [this]() {
        Audio::GetInstance()->Play("select_enter");
        };
    UI::InteractionReaction reactions = { .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f}, .custom = startButtonReaction };
    m_startButton->SetOnSelectedReaction(reactions);
    m_startButton->SetOnPressedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f} });
    startButtonReaction = [this]() {
        m_pressUI = "start";
        Audio::GetInstance()->Play("select_enter");
        };
    m_startButton->SetActiveReaction(startButtonReaction);
    m_startButton->ShowThisFrame();

    std::unique_ptr<UI::Element> m_exitButton;
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
    m_exitButton->SetOnPressedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f} });
    exitButtonReaction = [this]() {
        Audio::GetInstance()->Play("select_enter");
        m_pressUI = "exit";
        };
    m_exitButton->SetActiveReaction(exitButtonReaction);
    m_exitButton->ShowThisFrame();


    m_selectionGroup = std::make_unique<UI::SelectionGroup>();
    m_selectionGroup->SetInput(m_pInput);
    m_selectionGroup->Add(std::move(m_startButton));
    m_selectionGroup->Add(std::move(m_exitButton));
    m_selectionGroup->SetMoveUpBinding(UI::InputTrigger{ .key = DIK_W });
    m_selectionGroup->SetMoveDownBinding(UI::InputTrigger{ .key = DIK_S });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_RETURN, .mouseButton = MOUSE_LEFT, .controller = Controller::A });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_SPACE });
    m_selectionGroup->SetUsableCount(1);
}

TitleSceneUI::~TitleSceneUI()
{}

void TitleSceneUI::Update(const TitleSceneScreen& screen) {

    switch (screen)
    {
    case TitleSceneScreen::BootScreen:
        m_pressAnyKeySprite->Update();
        if (m_pInput->PressAnyKey() || m_pInput->PressAnyButton() || m_pInput->TriggerMouse(0) || m_pInput->TriggerMouse(1))
        {
            m_pressUI = "bootScreen";
            Audio::GetInstance()->Play("select_enter");
        }
        break;

    case TitleSceneScreen::TitleScreen:

        m_selectionGroup->Update();

        break;
    }
}

void TitleSceneUI::DrawBootScreen() {
    m_pressAnyKeySprite->Draw();
}

void TitleSceneUI::DrawTitleScreen() {
    m_selectionGroup->Draw();
}
