#include "PlayerBaseState.h"
#pragma once

class CrouchState : public PlayerBaseState {
public:
    PlayerStateId GetStateId() const noexcept override { return PlayerStateId::Crouch; }
    void Enter(Player& player) override;
    void Update(Player& player) override;
    void Exit(Player& player) override;
};

