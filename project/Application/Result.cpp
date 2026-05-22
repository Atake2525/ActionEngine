#include "Result.h"
#include "GameTime.h"
#include "kMath.h"
#include "TextureManager.h"
#include <string>
#include "WinApp.h"
#include "EasingUtility.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "FadeManager.h"
#include "Collision.h"
#include "MouseCursor.h"
#include "Audio.h"


Result::~Result()
{
    m_goalTimeNumbersArray.clear();
    m_clearTimeSprites.clear();
}

void Result::Initialize()
{

    m_pInput = Input::GetInstance();

    // 0 ~ 9 までのSpriteをあらかじめ読み込んでおく
    for (int i = 0; i < 10; i++)
    {
        std::string str = std::to_string(i);
        TextureManager::GetInstance()->LoadTexture("Resources/Sprite/Result/" + str + ".png");
    }
    m_windowSize = WinApp::GetInstance()->GetWindowSize();
    DirectX::TexMetadata metaData = TextureManager::GetInstance()->GetMetaData("Resources/Sprite/Result/0.png");
    m_timeTextureScale = { static_cast<float>(metaData.width), static_cast<float>(metaData.height) };

    // ステージクリアのテキスト用スプライトの用意
    m_clearTextSprite = std::make_unique<Sprite>();
    m_clearTextSprite->Initialize("Resources/Sprite/Result/StageClearText.png");
    Vector2 textureScale = m_clearTextSprite->GetScale();
    m_clearTextSprite->SetScale(Vector2::Zero);
    m_clearTextSprite->SetAnchorPoint(ANCHORPOINT_MIDDLETOP);
    m_clearTextSprite->SetPosition({ m_windowSize.x / 2.0f, 0.0f });
    metaData = TextureManager::GetInstance()->GetMetaData("Resources/Sprite/Result/StageClearText.png");
    m_clearTextTextureScale = textureScale;

    // リザルト背景用の黒いスプライトを用意
    m_backScreenSprite = std::make_unique<Sprite>();
    m_backScreenSprite->Initialize("Resources/Sprite/black1x1.png");
    m_backScreenSprite->SetScale(Vector2::Zero);
    m_backScreenSprite->SetAnchorPoint(ANCHORPOINT_MIDDLE);
    m_backScreenSprite->SetPosition({ m_windowSize.x / 2.0f, m_windowSize.y / 1.8f });
    m_backScreenSprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.4f });

    m_clearTimeTextSprite = std::make_unique<Sprite>();
    m_clearTimeTextSprite->Initialize("Resources/Sprite/Result/StageClearTime.png");
    m_clearTimeTextureScale = m_clearTimeTextSprite->GetScale() / 1.7f;
    m_clearTimeTextSprite->SetScale(Vector2::Zero);
    m_clearTimeTextSprite->SetAnchorPoint(ANCHORPOINT_LEFTTOP);
    m_clearTimeTextSprite->SetPosition(m_textMarginRatio + m_backScreenSprite->GetPosition() - (m_windowSize * m_backScreenRatio / 2.0f));


    // リザルトUIで使うスプライトを用意
    const Vector2 basePosition = m_backScreenSprite->GetPosition() + Vector2{ 0.0f, m_windowSize.y * 0.12f };
    const float horizontalOffset = m_windowSize.x * 0.16f;

    std::function<void()> retryFunc = [&]() {
        std::function<void()> sceneFunc = [&]() {
            SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
            };
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(1.0f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(sceneFunc);
        };
    std::function<void()> goTitleFunc = [&]() {
        std::function<void()> sceneFunc = [&]() {
            SceneManager::GetInstance()->SetNextScene("TITLE");

            };
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(1.0f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(sceneFunc);
        };

    UI::InteractionReaction reaction;
    reaction.scale = true;
    reaction.scaleAmount = { 1.1f, 1.1f };
    reaction.highlightColor = { 0.0f, 1.0f, 0.6f, 1.0f };
    reaction.highlight = true;
    reaction.custom = [this]() {
        Audio::GetInstance()->Play("select");
        };
    std::array<std::unique_ptr<UI::Element>, 2> m_uiElements;
    for (int i = 0; i < m_uiElements.size(); i++)
    {
        m_uiElements[i] = std::make_unique<UI::Button>();
        m_uiElements[i]->SetOnSelectedReaction(reaction);
    }
    m_uiElements[0]->Initialize("Resources/Sprite/Result/GoTitle.png", *m_pInput);
    m_uiElements[0]->SetPosition({ basePosition.x + horizontalOffset * -1.2f, basePosition.y });
    m_uiElements[0]->SetActiveReaction(goTitleFunc);
    Vector2 size = m_uiElements[0]->GetScale();
    m_uiElements[0]->SetScale(size * 0.8f);

    m_uiElements[1]->Initialize("Resources/Sprite/Result/ReTry.png", *m_pInput);
    m_uiElements[1]->SetPosition({ basePosition.x + horizontalOffset * 1.2f, basePosition.y });
    m_uiElements[1]->SetActiveReaction(retryFunc);
    size = m_uiElements[1]->GetScale();
    m_uiElements[1]->SetScale(size * 0.8f);

    // UIの選択グループを作成
    m_uiSelectionGroup = std::make_unique<UI::SelectionGroup>();
    m_uiSelectionGroup->SetInput(m_pInput);
    UI::InputTrigger trigger;
    trigger.controller = Controller::A;
    trigger.key = DIK_SPACE;
    trigger.mouseButton = MOUSE_LEFT;
    // UI全体のインタラクトバインドを設定
    m_uiSelectionGroup->SetAllInteractBinding(trigger);
    trigger.Reset();
    trigger.key = DIK_RETURN;
    m_uiSelectionGroup->SetAllInteractBinding(trigger);
    // UIの移動バインドを設定
    m_uiSelectionGroup->SetMoveDownBinding({ .key = DIK_D, .dpad = DPad::Right });
    m_uiSelectionGroup->SetMoveDownBinding({ .key = DIK_RIGHT });
    m_uiSelectionGroup->SetMoveUpBinding({ .key = DIK_A, .dpad = DPad::Left });
    m_uiSelectionGroup->SetMoveUpBinding({ .key = DIK_LEFT });
    // UI要素を選択グループに追加
    m_uiSelectionGroup->Add(move(m_uiElements[0]));
    m_uiSelectionGroup->Add(move(m_uiElements[1]));
    // UIの使用可能数の設定
    m_uiSelectionGroup->SetUsableCount(1);

    m_playTimer = 0.0f;
}

void Result::Update()
{
    switch (m_resultPhase)
    {
    case Result::ResultDrawPhase::backScreen:
        if (!m_isGoal)
        {
            m_playTimer += GameTime::GetInstance()->GetUnscaledDeltaTime();
        }
        else
        {
            // アニメーションさせるためにタイマーを使う
            m_resultDrawTimer += GameTime::GetInstance()->GetUnscaledDeltaTime();
            m_resultDrawTimer = std::clamp(m_resultDrawTimer, 0.0f, 1.0f);
            for (int i = 0; i < m_clearTimeSprites.size(); i++)
            {
                m_clearTimeSprites[i]->Update();
            }
            float resultTimer = EaseOutQuint(0.0f, 1.0f, m_resultDrawTimer);
            m_clearTextSprite->SetScale(m_clearTextTextureScale * resultTimer);
            m_clearTextSprite->Update();
            m_clearTimeTextSprite->SetScale(m_clearTimeTextureScale * resultTimer);
            m_clearTimeTextSprite->SetPosition(m_textMarginRatio + m_backScreenSprite->GetPosition() - m_backScreenSprite->GetScale() / 2.0f);
            m_clearTimeTextSprite->Update();
            m_backScreenSprite->SetScale((m_windowSize * m_backScreenRatio) * resultTimer);
            m_backScreenSprite->Update();

            if (resultTimer == 1.0f)
            {
                m_resultDrawTimer = 0.0f;
                CalculateStageClearTimer();
                m_resultPhase = ResultDrawPhase::clearTime;
            }

        }
        break;
    case Result::ResultDrawPhase::clearTime:
        m_resultDrawTimer += GameTime::GetInstance()->GetUnscaledDeltaTime();
        if (m_resultDrawTimer >= 0.3f)
        {
            m_resultPhase = ResultDrawPhase::ui;
            m_pInput->ShowMouseCursor(true);
        }
        break;
    case Result::ResultDrawPhase::ui:
        m_uiSelectionGroup->Update();

        break;
    }

}

void Result::Draw()
{
    m_backScreenSprite->Draw();
    m_clearTextSprite->Draw();
    m_clearTimeTextSprite->Draw();
    for (const auto& clearTimeSprite : m_clearTimeSprites)
    {
        clearTimeSprite->Draw();
    }
    if (m_resultPhase == ResultDrawPhase::ui)
    {
        m_uiSelectionGroup->ShowThisFrame();
        m_uiSelectionGroup->Draw();
    }
    else
    {
        m_uiSelectionGroup->HideThisFrame();
    }
}

void Result::StageClear()
{
    m_isGoal = true;
}

void Result::CalculateStageClearTimer()
{

    // プレイ時間を時分秒に正規化する
    int totalCentiseconds = std::max(0, static_cast<int>(m_playTimer * 100.0f + 0.5f));
    int hours = totalCentiseconds / 360000;
    int minutes = (totalCentiseconds / 6000) % 60;
    int seconds = (totalCentiseconds / 100) % 60;
    int centiseconds = totalCentiseconds % 100;

    int displayTime = centiseconds + seconds * 100 + minutes * 10000 + hours * 1000000;

    // リザルト(クリアタイムの計算)
    // プレイ時間を桁数ごとに分割する
    int digit = GetDigitCount(static_cast<float>(displayTime));
    m_goalTimeNumbersArray.resize(digit);
    float timer = 0.0f;
    for (int i = 0; i < m_goalTimeNumbersArray.size(); i++)
    {
        int time = (displayTime - int(timer)) / int(std::pow(10, i));
        time %= 10;
        m_goalTimeNumbersArray[i] = time;
        timer += time * static_cast<float>(std::pow(10, i));

        // 二桁ごとにコロンを入れる(タイムっぽくするために)
        if (i != 0 && i % 2 == 0)
        {
            // 桁数の値をSpriteで読み込む
            std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
            sprite->Initialize("Resources/Sprite/Result/TimerColon.png");
            m_clearTimeSprites.push_back(move(sprite));
            digit++;
        }

        // 桁数の値をSpriteで読み込む
        std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
        sprite->Initialize("Resources/Sprite/Result/" + std::to_string(time) + ".png");
        m_clearTimeSprites.push_back(move(sprite));
    }

    Vector2 spriteScale = m_timeTextureScale / 2.0f;
    // 背景に合わせて位置を決める
    Vector2 leftTop = m_backScreenSprite->GetPosition() - m_backScreenSprite->GetScale() / 2.0f;
    Vector2 pos = m_clearTimeTextSprite->GetPosition();
    // 位置を揃える
    for (int i = digit; i > 0; i--)
    {
        m_clearTimeSprites[i - 1]->SetPosition({ pos.x + m_clearTimeTextSprite->GetScale().x + spriteScale.x * (digit - i), pos.y });
        m_clearTimeSprites[i - 1]->SetScale(spriteScale);
        m_clearTimeSprites[i - 1]->Update();

    }
}