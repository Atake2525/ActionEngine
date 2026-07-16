#include "CrouchState.h"
#include "Player.h"

void CrouchState::Enter(Player& player) {

}

void CrouchState::Update(Player& player) {
    player.HorizontalMove(player.m_crounchSpeed, player.m_crouchAccelerationTime, player.m_groundDecelerationTime);
}

void CrouchState::Exit(Player& player) {

}