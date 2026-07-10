#include "PlayerState.h"
#pragma once

class WallRunState : public PlayerState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::WallRun; }
    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

