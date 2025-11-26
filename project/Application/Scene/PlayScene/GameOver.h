#include <memory>
#include "UI.h"
#include "Object3d.h"
#include "Input.h"

#pragma once

// ゲームオーバーシーン
class GameOver
{
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

private:
    std::unique_ptr<Sprite> backScreen;
    std::unique_ptr<Sprite> frame;
    std::unique_ptr<Sprite> space;

    std::unique_ptr<UI> restart;
    std::unique_ptr<UI> stageSelect;
    std::unique_ptr<UI> title;

    int selectNum = 0;

    float frameMoveTimer_ = 0.0f;
    float frameMoveTime_ = 0.2f;

    int animationNumber = 0;

    float moveTimer_ = 0.0f;
    float moveTime_ = 3.0f;

    bool push = false;

    Input* input = nullptr;
};

