#include "RunState.h"
#include "Player.h"

void RunState::Enter(Player& player) {

}

void RunState::Update(Player& player) {
    player.GroundMove(player.GetRunSpeed());
}

void RunState::Exit(Player& player) {

}