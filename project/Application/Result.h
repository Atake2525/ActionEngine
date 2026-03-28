#include <array>
#include <vector>
#include "Sprite.h"
#include <memory>

#pragma once

class MouseCursor;

class Result
{
public:
    ~Result();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(MouseCursor*  mouseCursor);

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
    // リザルトUIの入力処理
    void UpdateUISelect();
    // リザルトUIの決定処理
    void EnterSelectUI();

    enum class ResultDrawPhase {
        backScreen = 0,
        clearTime = 1,
        ui = 2,
    };
    ResultDrawPhase m_resultPhase = ResultDrawPhase::backScreen;

    // UIの選択項目
    enum class ResultSelect {
        title = 0,
        retry = 1,
    };
    ResultSelect m_resultSelect = ResultSelect::retry;

private:
    float m_playTimer = 0.0f;
    bool m_isGoal = false;
    bool m_calculatedResults = false;

    MouseCursor* m_mouseCursor = nullptr;

    // クリアタイムの分割用
    std::vector<int> m_goalTimeNumbersArray;
    // 背景用スプライト
    std::unique_ptr<Sprite> m_backScreenSprite;
    // キスト用スプライト ステージクリア
    std::unique_ptr<Sprite> m_clearTextSprite;
    Vector2 m_clearTextTextureScale = Vector2::Zero;
    // テキスト用スプライト クリアタイム
    std::unique_ptr<Sprite> m_clearTimeTextSprite;
    Vector2 m_clearTimeTextureScale = Vector2::Zero;
    // クリアタイムの数値用スプライト
    std::vector<std::unique_ptr<Sprite>> m_clearTimeSprites;
    Vector2 m_timeTextureScale = Vector2::Zero;

    Vector2 m_backScreenRatio = { 0.8f, 0.7f };
    Vector2 m_windowSize = Vector2::Zero;
    float m_resultDrawTimer = 0.0f;

    Vector2 m_textMarginRatio = { 0.01f, 0.01f };

    // リザルトUI用スプライト
    std::array<std::unique_ptr<Sprite>, 2> m_uiSprites;
    std::array<Vector2, 2> m_uiBaseScales = {};
    // UI表示後にマウスカーソルを表示したか
    bool m_isUiCursorVisible = false;
};

