#include "Pause.h"
#include "WinApp.h"
#include "Input.h"
#include "EasingUtility.h"
#include "GameTime.h"
#include "OffScreenRendering.h"
#include "ImGuiManager.h"
#include "SceneManager.h"

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
            if (!m_pause)
            {
                pauseUIs[i].position = EaseOutQuint(m_animTimer, pauseUIs[i].targetPosition[0], pauseUIs[i].targetPosition[1]);
                OffScreenRendering::GetInstance()->SetGrayscaleIntensity(min(m_animTimer * 1.2f, 0.4f));
                pauseUIs[i].sprite->SetColor({ color.x, color.y, color.z, m_animTimer });
            }
            else
            {
                pauseUIs[i].position = EaseOutQuint(m_animTimer, pauseUIs[i].targetPosition[1], pauseUIs[i].targetPosition[0]);
                OffScreenRendering::GetInstance()->SetGrayscaleIntensity(min(1.0f - m_animTimer * 1.2f, 0.4f));
                //pauseUIs[i].sprite->SetColor({ 1.0f, color.y, color.z, max(1.0f - m_animTimer * 2.0f, 0.0f) });
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
    if (m_pause)
    {

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
        m_pauseSelect = static_cast<PauseSelect>(num);

        m_animTimer += GameTime::GetInstance()->GetDeltaTime() / m_animTime;

        for (int i = 0; i < pauseUIs.size(); i++)
        {
            if (static_cast<int>(m_pauseSelect) == i)
            {
                pauseUIs[i].sprite->SetColor({0.0f, 1.0f, 0.0f, 1.0f});
                pauseUIs[i].sprite
            }
            else
            {
                pauseUIs[i].sprite->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f });
            }
            pauseUIs[i].sprite->Update();
        }
    }
    int num = static_cast<int>(m_pauseSelect);
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
