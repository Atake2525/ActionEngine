#include "JumpState.h"
#include "Player.h"
#include <memory>

void JumpState::Enter(Player& player) {
    player.JumpStart();
    
}

void JumpState::Update(Player& player) {
    player.HorizontalMove(player.m_airSpeed, player.m_airAccelerationTime, player.m_airDecelerationTime);
    return;
}

void JumpState::Exit(Player& player) {

}