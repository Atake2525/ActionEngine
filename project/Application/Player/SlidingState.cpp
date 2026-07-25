#include "SlidingState.h"
#include "CrouchState.h"
#include "Player.h"

void SlidingState::Enter(Player& player) {

}

void SlidingState::Update(Player& player) {
    player.ChangeState(std::make_unique<CrouchState>());
}

void SlidingState::Exit(Player& player) {

}