#include "RunState.h"
#include "Player.h"

void RunState::Enter(Player& player) {

}

void RunState::Update(Player& player) {
    player.HorizontalMove(player.m_runSpeed, player.m_runAccelerationTime, player.m_groundDecelerationTime);
}

void RunState::Exit(Player& player) {

}