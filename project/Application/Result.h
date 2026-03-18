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
    float m_playTimer = 0.0f;
    bool m_isGoal = false;

    // クリアタイムの分割用
    std::vector<int> m_goalTimeNumbersArray;

    std::vector<std::unique_ptr<Sprite>> m_Sprites;
};

