#include "JumpState.h"
#include "Player.h"
#include <memory>
#include "RunState.h"

void JumpState::Enter(Player& player) {
    player.JumpStart();
    
}

void JumpState::Update(Player& player) {
    player.ChangeState(std::make_unique<RunState>());
    return;
}

void JumpState::Exit(Player& player) {

}