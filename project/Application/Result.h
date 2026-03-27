#include <vector>
#include "Sprite.h"
#include <memory>

#pragma once

class Result
{
public:
    ~Result();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// ゴールした時に呼び出す
    /// </summary>
    void StageClear();

private:

    // ステージクリア時間の算出
    void CalculateStageClearTimer();

private:
    float m_playTimer = 0.0f;
    bool m_isGoal = false;
    bool m_calculatedResults = false;

    // クリアタイムの分割用
    std::vector<int> m_goalTimeNumbersArray;

    std::unique_ptr<Sprite> m_backScreenSprite;

    std::unique_ptr<Sprite> m_clearTextSprite;
    Vector2 m_clearTextTextureScale = Vector2::Zero;

    std::unique_ptr<Sprite> m_clearTimeTextSprite;
    Vector2 m_clearTimeTextureScale = Vector2::Zero;

    std::vector<std::unique_ptr<Sprite>> m_clearTimeSprites;
    Vector2 m_timeTextureScale = Vector2::Zero;

    Vector2 m_backScreenRatio = { 0.8f, 0.7f };
    Vector2 m_windowSize = Vector2::Zero;
    float m_resultDrawTimer = 0.0f;

    Vector2 m_textMarginRatio = { 0.01f, 0.01f };
};

