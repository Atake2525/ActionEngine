#include "WallRunState.h"
#include "Player.h"

void WallRunState::Enter(Player& player) {
    player.WallRunStart();
}

void WallRunState::Update(Player& player) {
    player.HorizontalMove(player.m_airSpeed, player.m_airAccelerationTime, player.m_airDecelerationTime);
    player.WallRun();
}

void WallRunState::Exit(Player& player) {

}