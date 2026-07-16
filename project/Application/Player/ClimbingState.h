#include "PlayerBaseState.h"
#pragma once

class ClimbingState : public PlayerBaseState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::Climbing; }
    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

