#include "PlayerState.h"
#pragma once

class WallJumpState : public PlayerState {
public:
    void Enter(Player& playr) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

