#include "Pause.h"
#include "WinApp.h"
#include "Input.h"
#include "EasingUtility.h"
#include "GameTime.h"
#include "OffScreenRendering.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "Audio.h"
#include "FadeManager.h"

using namespace std;

void Pause::Initialize() {
    Vector2 m_windowSize = { float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()) };

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
        pauseUIs[i].sprite->SetAnchorPoint({ 0.5f, 0.0f });
        // 初期位置の設定(スクリーン上に無ければ良い)
        pauseUIs[i].sprite->SetPosition({ m_windowSize.x * 0.5f, i * 20.0f });
        // 色を好きに変更するためにSpriteの色を白にしているため変更
        pauseUIs[i].sprite->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f });

        // ポーズの切り替えアニメーション用の位置決め
        pauseUIs[i].targetPosition[0] = { m_windowSize.x * 0.5f, -pauseUIs[i].sprite->GetScale().y };
        float size = (m_windowSize.y - m_outSize.y * 2.0f) / float(pauseUIs.size());
        float targetPosY = max(((m_windowSize.y - m_outSize.y * 2.0f) / float(pauseUIs.size())) * i + m_outSize.y, pauseUIs[i].sprite->GetScale().y * i);
        pauseUIs[i].targetPosition[1] = { m_windowSize.x * 0.5f, targetPosY };
    }

    Audio::GetInstance()->LoadMP3("Resources/sound/pause_select.mp3", "pause_select");
}

void Pause::Update() {

    if (Input::GetInstance()->TriggerKeyInt(DIK_ESCAPE) && !m_pauseAnim)
    {
        m_pauseAnim = !m_pauseAnim;
        m_animTimer = 0.0f;
        if (m_pause)
        {
            Input::GetInstance()->ShowMouseCursor(false);
        }
        else
        {
            Input::GetInstance()->ShowMouseCursor(true);
        }
    }

    // ポーズアニメーションが再生されていない限りこれ以上処理をしないようにreturn
    if (m_pauseAnim) {
        // アニメーションタイマーを進める
        m_animTimer += GameTime::GetInstance()->GetDeltaTime() / m_animTime;

        // それぞれのSpriteに対して位置を変える計算を行う
        for (int i = 0; i < pauseUIs.size(); i++)
        {
            // ポーズ状態へとプレイ画面へ戻るの2種類のタイプでeasingを行う
            Vector4 color = pauseUIs[i].sprite->GetColor();
            if (!m_pause) // ポーズ状態へ入るとき
            {
                pauseUIs[i].position = EaseOutQuint(pauseUIs[i].targetPosition[0], pauseUIs[i].targetPosition[1], m_animTimer);
                OffScreenRendering::GetInstance()->SetGrayscaleIntensity(min(m_animTimer * 1.2f, 0.4f));
                pauseUIs[i].sprite->SetColor({ color.x, color.y, color.z, m_animTimer });
            }
            else // ポーズ状態から出る時
            {
                pauseUIs[i].position = EaseOutQuint(pauseUIs[i].targetPosition[1], pauseUIs[i].targetPosition[0], m_animTimer);
                OffScreenRendering::GetInstance()->SetGrayscaleIntensity(min(1.0f - m_animTimer * 1.2f, 0.4f));
                pauseUIs[i].sprite->SetColor({ color.x, color.y, color.z, EaseOutQuint(m_animTimer, 1.0f, 0.0f)});
            }

            pauseUIs[i].sprite->SetPosition(pauseUIs[i].position);
            pauseUIs[i].sprite->Update();
        }

        // アニメーションタイマーが1.0fを超えたら終了
        if (m_animTimer >= 1.0f)
        {
            m_pause = !m_pause;
            m_pauseAnim = false;
            m_animTimer = 0.0f;
        }
    }

    // ポーズ中の処理
    if (m_pause && !m_pauseAnim)
    {
        // キー入力処理
        int num = static_cast<int>(m_pauseSelect);
        num += (Input::GetInstance()->TriggerKeyInt(DIK_S) | Input::GetInstance()->TriggerKeyInt(DIK_DOWN));
        num -= (Input::GetInstance()->TriggerKeyInt(DIK_W) | Input::GetInstance()->TriggerKeyInt(DIK_UP));
        if (num < 0)
        {
            num = pauseUIs.size() - 1;
        }
        else if (num > pauseUIs.size() - 1)
        {
            num = 0;
        }

        if (num != static_cast<int>(m_pauseSelect))
        {
            m_selectNumberPre = static_cast<int>(m_pauseSelect);
            Audio::GetInstance()->Play("pause_select");
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
            Vector3 color = {
                       0.0f,
                       1.0f,
                       EaseOutQuint(m_animTimer, 1.0f, 0.0f)
            };
            pauseUIs[static_cast<int>(m_pauseSelect)].sprite->SetColor({ color.x, color.y, color.z, 1.0f });
            pauseUIs[static_cast<int>(m_pauseSelect)].sprite->Update();
            
            pauseUIs[static_cast<int>(m_selectNumberPre)].sprite->SetColor({0.0f, 1.0f, 1.0f, 1.0f });
            pauseUIs[static_cast<int>(m_selectNumberPre)].sprite->Update();

            if (m_animTimer >= 1.0f)
            {
                m_changeSelectAnim = false;
            }
        }
        else
        {
            if (Input::GetInstance()->TriggerKeyInt(DIK_RETURN) || Input::GetInstance()->TriggerMouse(0))
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
    switch (m_pauseSelect)
    {
    case PauseSelect::back:
        m_pauseAnim = !m_pauseAnim;
        m_animTimer = 0.0f;
        if (m_pause)
        {
            Input::GetInstance()->ShowMouseCursor(false);
        }
        else
        {
            Input::GetInstance()->ShowMouseCursor(true);
        }
        break;
    case PauseSelect::restart:
        OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
        SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
        break;
    case PauseSelect::stageSelect:
        break;
    case PauseSelect::setting:
        break;
    case PauseSelect::title:
        SceneManager::GetInstance()->SetNextScene("TITLE");
        break;
    }
}
