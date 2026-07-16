#include "PlayerBaseState.h"
#pragma once

class AirControlState : public PlayerBaseState
{
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::AirControl; }

    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

