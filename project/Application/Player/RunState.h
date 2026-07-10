#include "PlayerState.h"
#pragma once

class RunState : public PlayerState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::Run; }
    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

