#include "TitleSceneUI.h"
#include "Collision.h"
#include "Audio.h"

TitleSceneUI::TitleSceneUI(MouseCursor* mouseCursor) {
    // STARTとEXITのUIスプライトの初期化
    for (auto& ui : m_uiSprites)
    {
        ui = std::make_unique<Sprite>();
    }
    m_uiSprites[0]->Initialize("Resources/Sprite/UI/press_any_key.png");
    Vector2 size = m_uiSprites[0]->GetTextureSize();
    m_uiBaseScale[0] = { size.x * 0.3f, size.y * 0.3f };
    m_uiSprites[0]->SetScale({ size.x * 0.3f, size.y * 0.3f });
    m_uiSprites[0]->SetAnchorPoint({ 0.5f, 1.0f });
    Vector2 windowSize = { WinApp::GetInstance()->GetWindowSize() };
    m_uiSprites[0]->SetPosition({ windowSize.x * 0.6f, windowSize.y * 1.014f });
    m_uiSprites[0]->SetRotatioin(-SwapRadian(1.0f));
    m_uiSprites[0]->Update();
    m_uiSprites[1] = std::make_unique<Sprite>();
    m_uiSprites[1]->Initialize("Resources/Sprite/UI/ui_start.png");
    size = m_uiSprites[1]->GetTextureSize();
    m_uiBaseScale[1] = { size.x, size.y};
    m_uiSprites[1]->SetAnchorPoint({ 0.5f, 0.5f });
    m_uiSprites[1]->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f });
    m_uiSprites[1]->Update();
    m_uiSprites[2] = std::make_unique<Sprite>();
    m_uiSprites[2]->Initialize("Resources/Sprite/UI/ui_exit.png");
    size = m_uiSprites[2]->GetTextureSize();
    m_uiBaseScale[2] = { size.x, size.y };
    m_uiSprites[2]->SetAnchorPoint({ 0.5f, 0.5f });
    m_uiSprites[2]->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f + m_uiBaseScale[2].y * 1.2f });
    m_uiSprites[2]->Update();

    m_pInput = Input::GetInstance();
    m_mouseCursor = mouseCursor;

    m_startButton = std::make_unique<UI::Button>();
    m_startButton->Initialize("Resources/Sprite/UI/ui_start.png", *m_pInput);
    m_startButton->SetPosition({ windowSize.x * 0.5f, windowSize.y * 0.5f });
    // 音を鳴らす
    std::function<void()> startButtonReaction = [this]() {
        Audio::GetInstance()->Play("select_enter");
        };
    m_startButton->SetOnHoverReaction(UI::ButtonReaction{ .highlight = true, .highlightColor = {0.0f, 1.0f, 0.6f, 1.0f}, .scale = true, .scaleAmount = {1.1f, 1.1f}, .custom = startButtonReaction });
    m_startButton->SetOnClickReaction(UI::ButtonReaction{ .highlight = true, .highlightColor = {1.0f, 1.0f, 0.0f, 1.0f}, .scale = false, .custom = startButtonReaction });
    m_startButton->ShowThisFrame();
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
        Vector2 cursorPosition = m_mouseCursor->GetCursorPos();
        Vector3 pos = { cursorPosition.x, cursorPosition.y, 0.0f };
        AABB aabb = { {pos},{pos} };

        m_startButton->Update();

        // UIにマウスカーソルが入っている時、クリックしたときの処理
        /*if (CollisionSprite(m_uiSprites[1]->GetAABB(), aabb))
        {
            if (m_uiSprites[1]->GetColor().x != 0.0f)
            {
                Audio::GetInstance()->Play("select");
            }
            m_uiSprites[1]->SetColor({ 0.0f, 1.0f, 0.6f, 1.0f });
            m_uiSprites[1]->SetScale(m_uiBaseScale[1] * 1.1f);

            if (m_pInput->TriggerMouse(0) || m_pInput->TriggerKey(DIK_SPACE) || m_pInput->TriggerKey(DIK_RETURN))
            {
                m_pressUI = "start";
                Audio::GetInstance()->Play("select_enter");
            }
        }
        else
        {
            m_uiSprites[1]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            m_uiSprites[1]->SetScale(m_uiBaseScale[1]);
        }*/

        // UIにマウスカーソルが入っている時、クリックしたときの処理
        /*if (CollisionSprite(m_uiSprites[2]->GetAABB(), aabb))
        {
            if (m_uiSprites[2]->GetColor().x != 0.0f)
            {
                Audio::GetInstance()->Play("select");
            }
            m_uiSprites[2]->SetColor({ 0.0f, 1.0f, 0.6f, 1.0f });
            m_uiSprites[2]->SetScale(m_uiBaseScale[2] * 1.1f);

            if (m_pInput->TriggerMouse(0) || m_pInput->TriggerKey(DIK_SPACE) || m_pInput->TriggerKey(DIK_RETURN))
            {
                m_pressUI = "exit";
                Audio::GetInstance()->Play("select_enter");
            }
        }
        else
        {
            m_uiSprites[2]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            m_uiSprites[2]->SetScale(m_uiBaseScale[2]);
        }*/
        break;
    }

    for (const auto& ui : m_uiSprites)
    {
        ui->Update();
    }
}

void TitleSceneUI::DrawBootScreen() {
    m_uiSprites[0]->Draw();
}

void TitleSceneUI::DrawTitleScreen() {
   /* for (int i = 1; i < m_uiSprites.size(); i++)
    {
        m_uiSprites[i]->Draw();
    }*/
    m_startButton->Draw();
}