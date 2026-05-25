#include "Pause.h"
#include "WinApp.h"
#include "EasingUtility.h"
#include "GameTime.h"
#include "OffScreenRendering.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "Audio.h"
#include "FadeManager.h"
#include "MouseCursor.h"
#include "Collision.h"
#include <functional>

using namespace std;

void Pause::Initialize() {
    m_windowSize = WinApp::GetInstance()->GetWindowSize();

    m_input = Input::GetInstance();

    m_outSize.x = m_windowSize.x * 0.1f;
    m_outSize.y = m_windowSize.y * 0.1f;

    SetupUI();
}

void Pause::Update() {

    if (m_input->TriggerKeyInt(DIK_ESCAPE))
    {
        m_pauseAnim = true;
        m_pause = !m_pause;
        //m_animTimer = 0.0f;
        if (m_pause)
        {
            //m_mouseCursor->SetShowCursor(true);
            //m_mouseCursor->SetCursorPosition({ m_windowSize.x / 2.0f, m_windowSize.y / 4.5f });
            m_selectionGroup->Show();
        }
        else
        {
            //m_mouseCursor->SetShowCursor(false);
            m_selectionGroup->Hide();
        }
    }

    // ポーズアニメーションが再生されていない限りこれ以上処理をしないようにreturn
    if (m_pauseAnim) {
        // アニメーションタイマーを進める
        if (m_pause)
        {
            m_animTimer += GameTime::GetInstance()->GetDeltaTime() / m_animTime;
        }
        else
        {
            m_animTimer -= GameTime::GetInstance()->GetDeltaTime() / m_animTime;
        }
        m_animTimer = std::clamp(m_animTimer, 0.0f, 1.0f);

        
    }

    m_selectionGroup->Update();

}

void Pause::Draw() {
    m_selectionGroup->Draw();
}



void Pause::SetupUI() {

    m_selectionGroup = std::make_unique<UI::SelectionGroup>();
    m_selectionGroup->SetInput(m_input);

    std::array<std::unique_ptr<UI::Element>, 5> ui;

    for (int i = 0; i < ui.size(); i++)
    {
        // 設定するpngを調べる
        ui[i] = make_unique<UI::Button>();
        string str;

        switch (i) {
        case 0:
            ui[0]->Initialize("Resources/Sprite/Pause/back.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                Audio::GetInstance()->Play("select_enter");
                m_pause = false;
                m_selectionGroup->Hide();
                });
            break;
        case 1:
            ui[1]->Initialize("Resources/Sprite/Pause/restart.png", *m_input);
            ui[i]->SetActiveReaction([]() {
                std::function<void()> restartFunc = []() {
                    OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
                    SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
                    };
                Audio::GetInstance()->Play("select_enter");
                FadeManager::GetInstance()->FadeOut(0.5f);
                FadeManager::GetInstance()->SetFinishedFadeFunction(restartFunc);
                });
            break;
        case 2:
            ui[2]->Initialize("Resources/Sprite/Pause/setting.png", *m_input);
            ui[i]->SetActiveReaction([]() {
                Audio::GetInstance()->Play("select_cancel");
                });
            break;
        case 3:
            ui[3]->Initialize("Resources/Sprite/Pause/stageselect.png", *m_input);
            ui[i]->SetActiveReaction([]() {
                Audio::GetInstance()->Play("select_cancel");
                });
            break;
        case 4:
            ui[4]->Initialize("Resources/Sprite/Pause/title.png", *m_input);
            ui[i]->SetActiveReaction([]() {
                std::function<void()> goTitleFunc = []() {
                    SceneManager::GetInstance()->SetNextScene("TITLE");
                    OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
                    };
                Audio::GetInstance()->Play("select_enter");
                FadeManager::GetInstance()->FadeOut(0.5f);
                FadeManager::GetInstance()->SetFinishedFadeFunction(goTitleFunc);
                });
            break;
        }
        // 初期位置の設定(スクリーン上に無ければ良い)
        ui[i]->SetPosition({ m_windowSize.x * 0.5f, -ui[i]->GetScale().y });
        ui[i]->SetOnSelectedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor{0.0f, 1.0f, 0.6f, 1.0f} });

        // ポーズの切り替えアニメーション用の位置決め
        Vector2 exitPos = { m_windowSize.x * 0.5f, -ui[i]->GetScale().y };
        float size = (m_windowSize.y - m_outSize.y * 2.0f) / float(ui.size());
        float targetPosY = max(((m_windowSize.y - m_outSize.y * 2.0f) / float(ui.size())) * i + m_outSize.y, ui[i]->GetScale().y * 1.2f * i);
        Vector2 enterPos = { m_windowSize.x * 0.5f, m_windowSize.y / 12.0f + targetPosY };

        Vector2 firstPos = Vector2::Zero;
        float animTimer = 0.0f;
        float animTime = 0.4f;
        ui[i]->SetEnterReaction([firstPos, enterPos, animTimer = 0.0f, animTime = 0.4f, firstFrame = false](UI::Element& element) mutable {
            if (!firstFrame)
            {
                firstPos = element.GetPosition();
                firstFrame = true;
            }
            animTimer += GameTime::GetInstance()->GetUnscaledDeltaTime() / animTime;
            animTimer = std::clamp(animTimer, 0.0f, 1.0f);
            element.SetPosition(EaseOutQuint(firstPos, enterPos, animTimer));
            if (animTimer >= 1.0f)
            {
                animTimer = 0.0f;
                firstFrame = false;
                element.ShowThisFrame();
            }
            });
        ui[i]->SetExitReaction([firstPos, exitPos, animTimer = 0.0f, animTime = 0.4f, firstFrame = false](UI::Element& element) mutable {
            if (!firstFrame)
            {
                firstPos = element.GetPosition();
                firstFrame = true;
            }
            animTimer += GameTime::GetInstance()->GetUnscaledDeltaTime() / animTime;
            animTimer = std::clamp(animTimer, 0.0f, 1.0f);
            element.SetPosition(EaseOutQuint(firstPos, exitPos, animTimer));
            if (animTimer >= 1.0f)
            {
                animTimer = 0.0f;
                firstFrame = false;
                element.HideThisFrame();
            }
            });

        m_selectionGroup->Add(move(ui[i]));
    }
    m_selectionGroup->SetMoveUpBinding(UI::InputTrigger{ .key = DIK_W, .dpad = DPad::Up });
    m_selectionGroup->SetMoveUpBinding(UI::InputTrigger{ .key = DIK_UP });
    m_selectionGroup->SetMoveDownBinding(UI::InputTrigger{ .key = DIK_S, .dpad = DPad::Down });
    m_selectionGroup->SetMoveDownBinding(UI::InputTrigger{ .key = DIK_DOWN, .dpad = DPad::Down });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_SPACE, .mouseButton = 0, .controller = Controller::A });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_RETURN });
}
