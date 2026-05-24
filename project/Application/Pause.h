#include "Sprite.h"
#include <memory>
#include <array>
#include <string>
#include "Input.h"
#include "UIButton.h"
#include "SelectionGroup.h"
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

    const bool IsPause()const { return m_pause || m_pauseAnim; }

private:

    void SetUIEnterReaction();
    void SetUIExitReaction();

    struct PauseSprite
    {
        std::unique_ptr<UI::Button> ui;
        Vector2 targetPosition[2];
    };

    std::array<PauseSprite, 5> m_pauseUIs;

    Input* m_input = nullptr;

    Vector2 m_windowSize = { 0.0f };

    int m_selectNumber = 0;
    int m_selectNumberPre = 0;

    bool m_pause = false;

    bool m_pauseAnim = false;
    bool m_changeSelectAnim = false;

    float m_animTimer = 0.0f;

    float m_animTime = 0.4f;

    Vector2 m_outSize = { 0.0f };

    // 選択されているときの移動値
    Vector2 m_selectingPos = { 0.0f, 10.0f };
    // 選択されているときのサイズの倍率
    float m_selectingScale = 1.2f;


/// 関数

    void Enter(int selectNumber);

};