#include "JumpState.h"
#include "Player.h"

void JumpState::Enter(Player& player) {

}

void JumpState::Update(Player& player) {
    player.GroundMove(player.GetRunSpeed());
}

void JumpState::Exit(Player& player) {

}