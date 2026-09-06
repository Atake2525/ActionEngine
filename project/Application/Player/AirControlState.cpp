#include "AirControlState.h"
#include "Player.h"

void AirControlState::Enter(Player& player) {

}

void AirControlState::Update(Player& player) {
    player.HorizontalMove(player.m_airSpeed, player.m_airAccelerationTime);
}

void AirControlState::Exit(Player& player) {

}
