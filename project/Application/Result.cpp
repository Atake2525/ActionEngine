#include "Result.h"
#include "GameTime.h"
#include "kMath.h"
#include "TextureManager.h"
#include <string>
#include "WinApp.h"
#include "EasingUtility.h"
#include "ImGuiManager.h"
#include "Input.h"
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

void Result::Initialize(MouseCursor* mouseCursor)
{
    m_mouseCursor = mouseCursor;
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

    // タイトルへ
    m_uiSprites[0] = std::make_unique<Sprite>();
    m_uiSprites[0]->Initialize("Resources/Sprite/Result/GoTitle.png");
    m_uiSprites[0]->SetAnchorPoint(ANCHORPOINT_MIDDLE);
    m_uiBaseScales[0] = m_uiSprites[0]->GetScale() * 0.8f;
    m_uiSprites[0]->SetScale(Vector2::Zero);
    m_uiSprites[0]->SetPosition({ basePosition.x + horizontalOffset * -1.0f, basePosition.y });
    m_uiSprites[0]->Update();
    // リトライ
    m_uiSprites[1] = std::make_unique<Sprite>();
    m_uiSprites[1]->Initialize("Resources/Sprite/Result/ReTry.png");
    m_uiSprites[1]->SetAnchorPoint(ANCHORPOINT_MIDDLE);
    m_uiBaseScales[1] = m_uiSprites[1]->GetScale() * 0.8f;
    m_uiSprites[1]->SetScale(Vector2::Zero);
    m_uiSprites[1]->SetPosition({ basePosition.x + horizontalOffset * 1.0f, basePosition.y });
    m_uiSprites[1]->Update();

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
            m_mouseCursor->SetShowCursor(true);
            m_mouseCursor->SetCursorPosition(m_windowSize / 2.0f);
        }
        break;
    case Result::ResultDrawPhase::ui:
        UpdateUISelect();
        for (int i = 0; i < m_uiSprites.size(); i++)
        {
            int selectNum = static_cast<int>(m_resultSelect);
            float scaleRatio = 1.0f;
            Vector4 color;
            // 選択されていたら色を変えてサイズを大きくする
            if (selectNum == i)
            {
                color = { 0.0f, 1.0f, 0.6f, 1.0f };
                scaleRatio = 1.1f;
            }
            else
            {
                color = { 1.0f, 1.0f, 1.0f, 0.8f };
                scaleRatio = 1.0f;
            }
            m_uiSprites[i]->SetScale(m_uiBaseScales[i] * scaleRatio);
            m_uiSprites[i]->SetColor(color);
            m_uiSprites[i]->Update();
        }
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
        for (const auto& sprite : m_uiSprites)
        {
            sprite->Draw();
        }
    }
}

void Result::StageClear()
{
    //CalculateStageClearTimer();
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

void Result::UpdateUISelect()
{
    Input* input = Input::GetInstance();

    int selectIndex = static_cast<int>(m_resultSelect);
    if (input->TriggerKey(DIK_W) || input->TriggerKey(DIK_UP))
    {
        selectIndex--;
        Audio::GetInstance()->Play("select");
    }
    if (input->TriggerKey(DIK_S) || input->TriggerKey(DIK_DOWN))
    {
        selectIndex++;
        Audio::GetInstance()->Play("select");
    }

    for (int i = 0; i < m_uiSprites.size(); i++)
    {
        if (CollisionUISprite(m_uiSprites[i]->GetAABB(), m_mouseCursor->GetCursorPos()))
        {
            if (selectIndex != i)
            {
                Audio::GetInstance()->Play("select");
            }
            selectIndex = i;
        }
    }

    if (selectIndex < 0)
    {
        selectIndex = static_cast<int>(m_uiSprites.size()) - 1;
    }
    else if (selectIndex >= static_cast<int>(m_uiSprites.size()))
    {
        selectIndex = 0;
    }
    m_resultSelect = static_cast<ResultSelect>(selectIndex);


    if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE) || input->TriggerMouse(0))
    {
        EnterSelectUI();
    }
}

void Result::EnterSelectUI()
{
    std::function<void()> retryFunc = [&]() {
        SceneManager::GetInstance()->SetNextScene(SceneManager::GetInstance()->GetSceneName());
        };
    std::function<void()> goTitleFunc = [&]() {
        SceneManager::GetInstance()->SetNextScene("TITLE");
        };
    // UIの選択結果に応じてシーンを切り替える
    switch (m_resultSelect)
    {
    case ResultSelect::retry:
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(1.0f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(retryFunc);
        break;
    case ResultSelect::title:
        Audio::GetInstance()->Play("select_enter");
        FadeManager::GetInstance()->FadeOut(1.0f);
        FadeManager::GetInstance()->SetFinishedFadeFunction(goTitleFunc);
        break;
    }
}
