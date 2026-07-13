#include "CrouchState.h"
#include "Player.h"

void CrouchState::Enter(Player& player) {

}

void CrouchState::Update(Player& player) {
    player.GroundMove(player.GetCrouchSpeed());
}

void CrouchState::Exit(Player& player) {

}