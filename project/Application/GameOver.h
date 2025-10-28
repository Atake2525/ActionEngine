#include <memory>
#include "UI.h"
#include "Object3d.h"
#include "Input.h"

#pragma once

class GameOver
{
public:
    void Initialize();

    void Update();

    void Draw();

private:
    std::unique_ptr<Sprite> backScreen;
    std::unique_ptr<Sprite> frame;

    std::unique_ptr<UI> restart;
    std::unique_ptr<UI> stageSelect;
    std::unique_ptr<UI> title;

    int selectNum = 0;

    float frameMoveTimer_ = 0.0f;
    float frameMoveTime_ = 0.2f;

    bool canSelect = false;

    float moveTimer_ = 0.0f;
    float moveTime_ = 3.0f;



    Input* input = nullptr;
};

