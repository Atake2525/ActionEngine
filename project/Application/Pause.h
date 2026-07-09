#include "Sprite.h"
#include <memory>
#include <array>
#include <string>
#include "Input.h"
#include "UIButton.h"
#include "UISelectionGroup.h"
#pragma once

struct AppContext;

class Pause
{
public:
    void SetContext(AppContext& context) { m_pContext = &context; }

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

    const bool IsPause()const { return m_pause; }

private:
    AppContext* m_pContext = nullptr;

    void TogglePauseMenu();

    void SetupUI();
    std::unique_ptr<UI::SelectionGroup> m_selectionGroup = nullptr;
    Input* m_input = nullptr;
    bool m_pause = false;

    // 初期位置の計算に使う変数
    Vector2 m_windowSize = { 0.0f };

    // 選択されているときの移動値
    Vector2 m_selectingPos = { 0.0f, 10.0f };
    // 選択されているときのサイズの倍率
    float m_selectingScale = 1.2f;

};
