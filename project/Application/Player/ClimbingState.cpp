#include "ClimbingState.h"
#include "Player.h"

void ClimbingState::Enter(Player& player) {
    player.StartClimbing();
}

void ClimbingState::Update(Player& player) {
    player.Climbing();
}

void ClimbingState::Exit(Player& player){
}