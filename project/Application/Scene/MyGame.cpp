#include "MyGame.h"
#include "ImGuiManager.h"

void MyGame::Initialize() {
    FrameWork::Initialize();
}

void MyGame::Update() {
    FrameWork::Update();
}

void MyGame::Draw() {
}

void MyGame::Finalize() {
    FrameWork::Finalize();
    FrameWork::LoopOut();
}

