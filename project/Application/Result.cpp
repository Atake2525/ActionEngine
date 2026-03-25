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
    m_timeTextureScale = { (2.0f / m_windowSize.x) * static_cast<float>(metaData.width), (2.0f / m_windowSize.y) * static_cast<float>(metaData.height) };

    // リザルト背景用の黒いスプライトを用意
    m_backScreenSprite = std::make_unique<Sprite>();
    m_backScreenSprite->Initialize("Resources/Sprite/black1x1.png");
    m_backScreenSprite->SetScale({ 0.0f, 0.0f });
    m_backScreenSprite->SetAnchorPoint({ 0.5f, 0.5f });
    m_backScreenSprite->SetPosition({ m_windowSize.x / 2.0f, m_windowSize.y / 2.2f });
    m_backScreenSprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.4f });
}

void Result::Update()
{
    if (!m_isGoal)
    {
        m_playTimer += GameTime::GetInstance()->GetUnscaledDeltaTime();
    }
    else
    {
        // アニメーションさせるためにタイマーを使う
        m_timer += GameTime::GetInstance()->GetUnscaledDeltaTime();
        m_timer = std::clamp(m_timer, 0.0f, 1.0f);
        if (m_timer == 1.0f && !m_calculatedResults)
        {
            CalculateStageClearTimer();
            m_calculatedResults = true;
        }
        for (int i = 0; i < m_clearTimeSprites.size(); i++)
        {
            m_clearTimeSprites[i]->Update();
        }
        Vector2 spriteScale = EaseOutQuint(Vector2{ 0.0f, 0.0f }, m_windowSize * m_backScreenRatio, m_timer);
        m_backScreenSprite->SetScale(spriteScale);
        m_backScreenSprite->Update();

    }

}

void Result::Draw()
{
    m_backScreenSprite->Draw();
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
    m_playTimer *= 100.0f;
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

        // 桁数の値をSpriteで読み込む
        std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
        sprite->Initialize("Resources/Sprite/Result/" + std::to_string(time) + ".png");
        m_clearTimeSprites.push_back(move(sprite));
    }

    // 位置を揃える
    for (int i = digit; i > 0; i--)
    {
        Vector2 spriteScale = m_clearTimeSprites[i - 1]->GetScale() * m_timeTextureScale * 3.0f;
        // 背景に合わせて位置を決める
        Vector2 leftTop = m_backScreenSprite->GetPosition() - m_backScreenSprite->GetScale() / 2.0f;
        m_clearTimeSprites[i - 1]->SetPosition({ (leftTop.x + (m_backScreenSprite->GetScale().x * m_textMarginRatio.x)) + spriteScale.x * (digit - i), leftTop.y + (m_backScreenSprite->GetScale().y * m_textMarginRatio.y) });
        m_clearTimeSprites[i - 1]->SetScale(spriteScale);
        m_clearTimeSprites[i - 1]->Update();

    }
}
