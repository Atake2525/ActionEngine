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
#include "EngineContext.h"
#include <functional>

using namespace std;

void Pause::Initialize() {
    AppContext& ctx = *m_pContext;
    // 処理に必要な値の取得
    m_windowSize = ctx.engine.platform.window.GetWindowSize();
    m_input = &ctx.engine.platform.input;

    // UIの初期化
    SetupUI();
}

void Pause::Update() {

    if (m_input->TriggerKeyInt(DIK_ESCAPE) || m_input->TriggerButton(Controller::Menu))
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

    // UIElementの定義
    std::array<std::unique_ptr<UI::Element>, 5> ui;

    for (int i = 0; i < ui.size(); i++)
    {
        // 各々初期化
        ui[i] = make_unique<UI::Button>();
        ui[i]->SetContext(m_pContext->game.spriteFactory);
        string str;

        switch (i) { // それぞれのpngを指定してリアクション設定
        case 0:
            ui[0]->Initialize("Resources/Sprite/Pause/back.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                m_pContext->engine.assets.audio.Play("select_enter");
                TogglePauseMenu();
                m_selectionGroup->Hide();
                });
            break;
        case 1:
            ui[1]->Initialize("Resources/Sprite/Pause/restart.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                std::function<void()> restartFunc = [this]() {
                    m_pContext->engine.graphics.offScreen.SetGrayscaleIntensity(0.0f);
                    m_pContext->game.sceneManager.SetNextScene(m_pContext->game.sceneManager.GetSceneName());
                    };
                m_pContext->engine.assets.audio.Play("select_enter");
                m_pContext->engine.presentation.fade.FadeOut(0.5f);
                m_pContext->engine.presentation.fade.SetFinishedFadeFunction(restartFunc);
                });
            break;
        case 2:
            ui[2]->Initialize("Resources/Sprite/Pause/setting.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                m_pContext->engine.assets.audio.Play("select_cancel");
                });
            break;
        case 3:
            ui[3]->Initialize("Resources/Sprite/Pause/stageselect.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                m_pContext->engine.assets.audio.Play("select_cancel");
                });
            break;
        case 4:
            ui[4]->Initialize("Resources/Sprite/Pause/title.png", *m_input);
            ui[i]->SetActiveReaction([this]() {
                std::function<void()> goTitleFunc = [this]() {
                    m_pContext->game.sceneManager.SetNextScene("TITLE");
                    m_pContext->engine.graphics.offScreen.SetGrayscaleIntensity(0.0f);
                    };
                m_pContext->engine.assets.audio.Play("select_enter");
                m_pContext->engine.presentation.fade.FadeOut(0.5f);
                m_pContext->engine.presentation.fade.SetFinishedFadeFunction(goTitleFunc);
                });
            break;
        }
        // 初期位置の設定(スクリーン上に無ければ良い)
        ui[i]->SetPosition({ m_windowSize.x * 0.5f, -ui[i]->GetScale().y });
        ui[i]->SetOnSelectedReaction(UI::InteractionReaction{ .highlight = true, .highlightColor{0.0f, 1.0f, 0.6f, 1.0f} });

        // ポーズの切り替えアニメーション用の位置決め
        Vector2 exitPos = { m_windowSize.x * 0.5f, -ui[i]->GetScale().y };
        float size = (m_windowSize.y - outSize.y * 2.0f) / float(ui.size());
        float targetPosY = (std::max)(((m_windowSize.y - outSize.y * 2.0f) / float(ui.size())) * i + outSize.y, ui[i]->GetScale().y * 1.2f * i);
        Vector2 enterPos = { m_windowSize.x * 0.5f, m_windowSize.y / 12.0f + targetPosY };

        Vector2 firstPos = Vector2::Zero;
        float animTimer = 0.0f;
        float animTime = 0.4f;
        // EnterとExitのReactionを設定
        ui[i]->SetEnterReaction([this, firstPos, enterPos, animTimer = 0.0f, animTime = 0.4f, firstFrame = false](UI::Element& element) mutable {
            if (!firstFrame)
            {
                firstPos = element.GetPosition();
                firstFrame = true;
            }
            animTimer += m_pContext->engine.platform.time.GetUnscaledDeltaTime() / animTime;
            animTimer = std::clamp(animTimer, 0.0f, 1.0f);
            element.SetPosition(EaseOutQuint(firstPos, enterPos, animTimer));
            if (animTimer >= 1.0f)
            {
                animTimer = 0.0f;
                firstFrame = false;
                element.ShowThisFrame();
            }
            });
        ui[i]->SetExitReaction([this, firstPos, exitPos, animTimer = 0.0f, animTime = 0.4f, firstFrame = false](UI::Element& element) mutable {
            if (!firstFrame)
            {
                firstPos = element.GetPosition();
                firstFrame = true;
            }
            animTimer += m_pContext->engine.platform.time.GetUnscaledDeltaTime() / animTime;
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
