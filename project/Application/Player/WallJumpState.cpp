#include "WallJumpState.h"
#include "Player.h"

void WallJumpState::Enter(Player& player) {
    player.WallJumpStart();
}

void WallJumpState::Update(Player& player) {

}

void WallJumpState::Exit(Player& player) {

}