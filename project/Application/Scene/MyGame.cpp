#include "MyGame.h"
#include "ImGuiManager.h"

void MyGame::Initialize() {
    FrameWork::Initialize();
}

void MyGame::Update() {
    FrameWork::Update();

    finished = FrameWork::LoopOut();
}

void MyGame::Draw() {

}

void MyGame::Finalize() {
    FrameWork::Finalize();
}
