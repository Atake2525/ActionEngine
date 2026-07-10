#include "PlayerState.h"
#pragma once

class ClimbingState : public PlayerState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::Climbing; }
    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

