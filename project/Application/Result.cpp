#include "Result.h"
#include "GameTime.h"
#include "kMath.h"
#include "TextureManager.h"
#include <string>
#include "WinApp.h"
#include "EasingUtility.h"
#include "ImGuiManager.h"

Result::~Result()
{
    m_goalTimeNumbersArray.clear();
    m_clearTimeSprites.clear();
}

void Result::Initialize()
{
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

    m_playTimer = 100.0f;
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
        break;
    }

}

void Result::Draw()
{
    m_backScreenSprite->Draw();
    m_clearTextSprite->Draw();
    m_clearTimeTextSprite->Draw();
    for (int i = 0; i < m_clearTimeSprites.size(); i++)
    {
        m_clearTimeSprites[i]->Draw();
    }
}

void Result::StageClear()
{
    //CalculateStageClearTimer();
    m_isGoal = true;
}

void Result::CalculateStageClearTimer()
{
    int time = m_playTimer;
    m_playTimer *= 100.0f;
    if (time >= 60)
    {
        int minites = time / 60.0f;
        time = std::fmod(time, 60);
        int playTime = static_cast<int>(m_playTimer) % 100;
        time += minites * 100;
        playTime += time * 100;
        m_playTimer = playTime;
    }
    // リザルト(クリアタイムの計算)
    // プレイ時間を桁数ごとに分割する
    int digit = GetDigitCount(m_playTimer);
    m_goalTimeNumbersArray.resize(digit);
    float timer = 0.0f;
    for (int i = 0; i < m_goalTimeNumbersArray.size(); i++)
    {
        int time = (m_playTimer - timer) / int(std::pow(10, i));
        time %= 10;
        m_goalTimeNumbersArray[i] = time;
        timer += time * (std::pow(10, i));

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
