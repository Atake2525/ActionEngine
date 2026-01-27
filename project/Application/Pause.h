#include "Sprite.h"
#include <memory>
#include <array>
#pragma once

class Pause
{
public:
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

    const bool& GetPause()const { return m_pause; }

private:

    std::array<std::unique_ptr<Sprite>, 5> sprites;

    bool m_pause = false;

    bool m_pauseAnim = false;

    float m_animTimer = 0.0f;

    float m_animTime = 1.0f;
};

