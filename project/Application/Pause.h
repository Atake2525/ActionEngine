#include "Sprite.h"
#include <memory>
#include <array>
#include <string>
#pragma once

enum class PauseSelect : uint8_t {
    back = 0,
    restart = 1,
    stageSelect = 2,
    setting = 3,
    title = 4,
};

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

    const bool& IsPause()const { return m_pause || m_pauseAnim; }

private:

    struct PauseSprite
    {
        std::unique_ptr<Sprite> sprite;
        Vector2 position;
        Vector2 targetPosition[2];
        Vector2 
    };

    std::array<PauseSprite, 5> pauseUIs;

    PauseSelect m_pauseSelect = PauseSelect::back;

    Vector2 m_windowSize = { 0.0f };

    bool m_pause = false;

    bool m_pauseAnim = false;

    float m_animTimer = 0.0f;

    float m_animTime = 0.8f;

    Vector2 m_outSize = { 0.0f };

    // 選択されているときの移動値
    Vector2 m_selectingPos = { 0.0f, 10.0f };
    // 選択されているときのサイズの倍率
    float m_selectingScale = 1.2f;

};