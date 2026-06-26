#include "WallRunState.h"
#include "Player.h"

void WallRunState::Enter(Player& player) {
    player.WallRunStart();
}

void WallRunState::Update(Player& player) {
    player.WallRun();
}

void WallRunState::Exit(Player& player) {

}