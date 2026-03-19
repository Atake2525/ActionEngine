#include "Result.h"
#include "GameTime.h"
#include "kMath.h"
#include "TextureManager.h"
#include <string>

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
}

void Result::Update()
{
	if (!m_isGoal)
	{
		m_playTimer += GameTime::GetInstance()->GetUnscaledDeltaTime();
	}
	else
	{
		for (int i = 0; i < m_clearTimeSprites.size(); i++)
		{
			m_clearTimeSprites[i]->Update();
		}
	}

}

void Result::Draw()
{
	for (int i = 0; i < m_clearTimeSprites.size(); i++)
	{
		m_clearTimeSprites[i]->Draw();
	}
}

void Result::StageClear()
{
	if (!m_isGoal)
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
			Vector2 spriteScale = m_clearTimeSprites[i - 1]->GetScale() * 0.5f;
			m_clearTimeSprites[i - 1]->SetPosition({ (WinApp::GetInstance()->GetkClientWidth() / 2.0f) + spriteScale.x * (digit - i), WinApp::GetInstance()->GetkClientHeight() / 2.0f });
			m_clearTimeSprites[i - 1]->SetScale(spriteScale);
			m_clearTimeSprites[i - 1]->Update();

		}
		m_isGoal = true;
	}
}
