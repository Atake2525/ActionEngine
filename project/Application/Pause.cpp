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
#include <cstddef>
#include <functional>

using namespace std;

void Pause::Initialize() {
    // 処理に必要な値の取得
    m_windowSize = WinApp::GetInstance()->GetWindowSize();
    m_input = Input::GetInstance();

    // UIの初期化
    SetupUI();
}

void Pause::Update() {

    if (m_input->TriggerKeyInt(DIK_ESCAPE))
    {
        // ポーズ移行処理を実行
        TogglePauseMenu();
    }
    // UIを更新
    m_selectionGroup->Update();

}

void Pause::Draw() {
    // UIを描画
    // UIクラス内で描画処理の切り替えを行っているため常にDrawを呼び出して大丈夫
    m_selectionGroup->Draw();
}

void Pause::TogglePauseMenu() {
    // ポーズフラグの切り替え
    m_pause = !m_pause;
    if (m_pause)
    {
        // マウスカーソルを表示してカーソルの位置を指定
        m_input->ShowMouseCursor(true);
        SetCursorPos(static_cast<int>(m_windowSize.x / 2.0f), static_cast<int>(m_windowSize.y / 4.5f));
        // enterReactionを再生
        m_selectionGroup->Show();
    }
    else
    {
        // マウスカーソルを非表示に戻す
        m_input->ShowMouseCursor(false);
        // exitReactionを再生
        m_selectionGroup->Hide();
    }
}

void Pause::SetupUI() { 
    // ウィンドウサイズの
    Vector2 outSize;
    outSize.x = m_windowSize.x * 0.1f;
    outSize.y = m_windowSize.y * 0.1f;

    // メンバ変数(SelectionGroup)の作成
    m_selectionGroup = std::make_unique<UI::SelectionGroup>();
    m_selectionGroup->SetInput(m_input);

    struct PauseMenuItem {
        const char* texturePath;
        std::function<void()> activeReaction;
    };

    constexpr std::size_t menuItemCount = 5;
    const std::array<PauseMenuItem, menuItemCount> menuItems = { {
        {
            "Resources/Sprite/Pause/back.png",
            [this]() {
                Audio::GetInstance()->Play("select_enter");
                TogglePauseMenu();
            }
        },
        {
            "Resources/Sprite/Pause/restart.png",
            []() {
                std::function<void()> restartFunc = []() {
                    OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
                    SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
                    };
                Audio::GetInstance()->Play("select_enter");
                FadeManager::GetInstance()->FadeOut(0.5f);
                FadeManager::GetInstance()->SetFinishedFadeFunction(restartFunc);
            }
        },
        {
            "Resources/Sprite/Pause/setting.png",
            []() {
                Audio::GetInstance()->Play("select_cancel");
            }
        },
        {
            "Resources/Sprite/Pause/stageselect.png",
            []() {
                Audio::GetInstance()->Play("select_cancel");
            }
        },
        {
            "Resources/Sprite/Pause/title.png",
            []() {
                std::function<void()> goTitleFunc = []() {
                    SceneManager::GetInstance()->SetNextScene("TITLE");
                    OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
                    };
                Audio::GetInstance()->Play("select_enter");
                FadeManager::GetInstance()->FadeOut(0.5f);
                FadeManager::GetInstance()->SetFinishedFadeFunction(goTitleFunc);
            }
        },
    } };

    // UIElementの定義
    std::array<std::unique_ptr<UI::Element>, menuItemCount> ui;

    for (std::size_t i = 0; i < ui.size(); i++)
    {
        // 各々初期化
        ui[i] = make_unique<UI::Button>();
        ui[i]->Initialize(menuItems[i].texturePath, *m_input);
        ui[i]->SetActiveReaction(menuItems[i].activeReaction);

        // 初期位置の設定(スクリーン上に無ければ良い)
        ui[i]->SetPosition({ m_windowSize.x * 0.5f, -ui[i]->GetScale().y });
        ui[i]->SetOnSelectedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor{0.0f, 1.0f, 0.6f, 1.0f} });

        // ポーズの切り替えアニメーション用の位置決め
        Vector2 exitPos = { m_windowSize.x * 0.5f, -ui[i]->GetScale().y };
        float targetPosY = max(((m_windowSize.y - outSize.y * 2.0f) / float(ui.size())) * i + outSize.y, ui[i]->GetScale().y * 1.2f * i);
        Vector2 enterPos = { m_windowSize.x * 0.5f, m_windowSize.y / 12.0f + targetPosY };

        Vector2 firstPos = Vector2::Zero;
        float animTimer = 0.0f;
        float animTime = 0.4f;
        // EnterとExitのReactionを設定
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
    // それぞれの操作設定
    m_selectionGroup->SetMoveUpBinding(UI::InputTrigger{ .key = DIK_W, .dpad = DPad::Up });
    m_selectionGroup->SetMoveUpBinding(UI::InputTrigger{ .key = DIK_UP });
    m_selectionGroup->SetMoveDownBinding(UI::InputTrigger{ .key = DIK_S, .dpad = DPad::Down });
    m_selectionGroup->SetMoveDownBinding(UI::InputTrigger{ .key = DIK_DOWN, .dpad = DPad::Down });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_SPACE, .mouseButton = 0, .controller = Controller::A });
    m_selectionGroup->SetInteractBinding(UI::InputTrigger{ .key = DIK_RETURN });
}
