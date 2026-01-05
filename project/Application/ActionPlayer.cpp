#include "ActionPlayer.h"
#include "Camera.h"
#include "CollisionManager.h"
#include "GameTime.h"
#include "JsonLoader.h"

#ifndef NDEBUG
    #include "ImGuiManager.h"
#endif // !NDEBUG


ActionPlayer::ActionPlayer() {

}

ActionPlayer::~ActionPlayer() {

}

void ActionPlayer::Initialize(Camera* camera) {
    m_pCamera = camera;

    m_moveVelocity = Vector3::Zero;
}

void ActionPlayer::Update() {

}

void ActionPlayer::Draw() {

}
