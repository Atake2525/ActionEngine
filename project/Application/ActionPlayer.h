#include "Transform.h"
#include <memory>
#include "Object3d.h"

#pragma once

class Camera;
class Input;

class ActionPlayer {

public:
    void Initialize();
    void Updaet();
    void Draw();
private:
    // 必須メンバ変数
    Input* M_pInput;
    Camera* M_pCamera;
    // 行動関数


};

