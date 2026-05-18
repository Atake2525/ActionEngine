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

void Pause::Initialize(MouseCursor* mouseCursor) {
    m_windowSize = WinApp::GetInstance()->GetWindowSize();

    m_input = Input::GetInstance();
    m_mouseCursor = mouseCursor;

    m_outSize.x = m_windowSize.x * 0.1f;
    m_outSize.y = m_windowSize.y * 0.1f;

    for (int i = 0; i < pauseUIs.size(); i++)
    {
        // 設定するpngを調べる
        pauseUIs[i].sprite = make_unique<Sprite>();
        PauseSelect select = static_cast<PauseSelect>(i);
        string str;
        switch (select)
        {
        case PauseSelect::back:
            str = "back";
            break;
        case PauseSelect::restart:
            str = "restart";
            break;
        case PauseSelect::stageSelect:
            str = "stageSelect";
            break;
        case PauseSelect::setting:
            str = "setting";
            break;
        case PauseSelect::title:
            str = "title";
            break;
        default:
            break;
        }

        // 諸々の初期化処理
        pauseUIs[i].sprite->Initialize("Resources/Sprite/Pause/" + str + ".png");
        // Spriteの中心位置を決める(横 : 中心, 縦 : 上)
        pauseUIs[i].sprite->SetAnchorPoint(ANCHORPOINT_MIDDLE);
        // 初期位置の設定(スクリーン上に無ければ良い)
        pauseUIs[i].sprite->SetPosition({ m_windowSize.x * 0.5f, -pauseUIs[i].sprite->GetScale().y });
        // 色を好きに変更するためにSpriteの色を白にしているため変更
        pauseUIs[i].sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

        pauseUIs[i].baseScale = { pauseUIs[i].sprite->GetScale() };

        // ポーズの切り替えアニメーション用の位置決め
        pauseUIs[i].targetPosition[0] = { m_windowSize.x * 0.5f, -pauseUIs[i].sprite->GetScale().y };
        float size = (m_windowSize.y - m_outSize.y * 2.0f) / float(pauseUIs.size());
        float targetPosY = max(((m_windowSize.y - m_outSize.y * 2.0f) / float(pauseUIs.size())) * i + m_outSize.y, pauseUIs[i].sprite->GetScale().y * 1.2f * i);
        pauseUIs[i].targetPosition[1] = { m_windowSize.x * 0.5f, m_windowSize.y / 12.0f + targetPosY };
    }

    pauseUIs[0].sprite->SetColor({ 0.0f, 1.0f, 0.6f, 1.0f });
}

void Pause::Update() {

    if (m_input->TriggerKeyInt(DIK_ESCAPE))
    {
        m_pauseAnim = true;
        m_pause = !m_pause;
        //m_animTimer = 0.0f;
        if (!m_pause)
        {
            m_mouseCursor->SetShowCursor(false);
            m_animTimer = 1.0f;
        }
        else
        {
            m_mouseCursor->SetShowCursor(true);
            m_mouseCursor->SetCursorPosition({ m_windowSize.x / 2.0f, m_windowSize.y / 4.5f });
            m_animTimer = 0.0f;
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

        // それぞれのSpriteに対して位置を変える計算を行う
        for (int i = 0; i < pauseUIs.size(); i++)
        {
            // ポーズ状態へとプレイ画面へ戻るの2種類のタイプでeasingを行う
            Vector4 color = pauseUIs[i].sprite->GetColor();
            pauseUIs[i].position = EaseOutQuint(pauseUIs[i].targetPosition[0], pauseUIs[i].targetPosition[1], m_animTimer);
            OffScreenRendering::GetInstance()->SetGrayscaleIntensity(min(m_animTimer * 1.2f, 0.4f));
            pauseUIs[i].sprite->SetColor({ color.x, color.y, color.z, m_animTimer });

            pauseUIs[i].sprite->SetPosition(pauseUIs[i].position);
            pauseUIs[i].sprite->Update();
        }

        // アニメーションタイマーが1.0fを超えたら終了
        if (m_animTimer == 1.0f || m_animTimer == 0.0f)
        {
            m_pauseAnim = false;
            //m_animTimer = 0.0f;
        }
    }

    // ポーズ中の処理
    if (m_pause && !m_pauseAnim)
    {
        // キー入力処理
        int num = static_cast<int>(m_pauseSelect);
        num += (m_input->TriggerKeyInt(DIK_S) | m_input->TriggerKeyInt(DIK_DOWN));
        num -= (m_input->TriggerKeyInt(DIK_W) | m_input->TriggerKeyInt(DIK_UP));

        for (int i = 0; i < pauseUIs.size(); i++)
        {
            if (CollisionUISprite(pauseUIs[i].sprite->GetTextureSize(), { 1.0f, 1.0f }, m_mouseCursor->GetCursorPos(), {1.0f, 1.0f}))
            {
                num = i;
            }
        }

        if (num < 0)
        {
            num = static_cast<int>(pauseUIs.size()) - 1;
        }
        else if (num > static_cast<int>(pauseUIs.size()) - 1)
        {
            num = 0;
        }

        if (num != static_cast<int>(m_pauseSelect))
        {
            m_selectNumberPre = static_cast<int>(m_pauseSelect);
            Audio::GetInstance()->Play("select");
            m_changeSelectAnim = true;
            m_animTimer = 0.0f;
        }

        // 入力による処理
        if (m_changeSelectAnim)
        {
            m_pauseSelect = static_cast<PauseSelect>(num);
            // タイマーを進める
            m_animTimer += GameTime::GetInstance()->GetDeltaTime() / m_animTime / 0.4f;

            // 選択されているUIに対して処理を行う
            pauseUIs[static_cast<int>(m_pauseSelect)].sprite->SetColor({ 0.0f, 1.0f, 0.6f, 1.0f });
            pauseUIs[static_cast<int>(m_pauseSelect)].sprite->SetScale(pauseUIs[static_cast<int>(m_pauseSelect)].baseScale * 1.1f);
            pauseUIs[static_cast<int>(m_pauseSelect)].sprite->Update();

            pauseUIs[static_cast<int>(m_selectNumberPre)].sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            pauseUIs[static_cast<int>(m_selectNumberPre)].sprite->SetScale(pauseUIs[static_cast<int>(m_selectNumberPre)].baseScale);
            pauseUIs[static_cast<int>(m_selectNumberPre)].sprite->Update();

            if (m_animTimer >= 1.0f)
            {
                m_changeSelectAnim = false;
            }
        }
        else
        {
            if (m_input->TriggerKeyInt(DIK_RETURN) || m_input->TriggerKeyInt(DIK_SPACE) || m_input->TriggerMouse(0))
            {
                Enter(static_cast<int>(m_pauseSelect));
            }
        }
    }
    m_selectNumber = static_cast<int>(m_pauseSelect);

}

void Pause::Draw() {
    if (m_pause || m_pauseAnim)
    {
        for (int i = 0; i < pauseUIs.size(); i++)
        {
            pauseUIs[i].sprite->Draw();
        }
    }
}

void Pause::Enter(int selectNumber) {
    std::function<void()> restartFunc = [&]() {
        OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
        SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
        };
    std::function<void()> goTitleFunc = [&]() {
        SceneManager::GetInstance()->SetNextScene("TITLE");
        OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
        };
    switch (m_pauseSelect)
    {
    case PauseSelect::back:
        Audio::GetInstance()->Play("select_enter");
        m_pauseAnim = !m_pauseAnim;
        m_pause = false;
        //m_animTimer = 0.0f;
        if (m_pause)
        {
            m_mouseCursor->SetShowCursor(true);
            m_mouseCursor->SetCursorPosition({ m_windowSize.x / 2.0f, m_windowSize.y / 4.5f });
        }
        else
        {
            m_mouseCursor->SetShowCursor(false);
        }
        break;
    case PauseSelect::restart:
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(0.5f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(restartFunc);
        break;
    case PauseSelect::stageSelect:
        Audio::GetInstance()->Play("select_cancel");
        break;
    case PauseSelect::setting:
        Audio::GetInstance()->Play("select_cancel");
        break;
    case PauseSelect::title:
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(0.5f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(goTitleFunc);
        break;
    }
}
