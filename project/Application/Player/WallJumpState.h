#include "PlayerState.h"
#pragma once

class WallJumpState : public PlayerState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::WallJump; }
    void Enter(Player& playr) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

