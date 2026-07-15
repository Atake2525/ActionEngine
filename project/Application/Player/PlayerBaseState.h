#pragma once

class Player;

// 移動タイプ
enum class PlayerStateId {
    Run,
    Crouch,
    Jump,
    WallRun,
    WallJump,
    Climbing,
    AirControl
};

class PlayerBaseState {
public:
    virtual ~PlayerBaseState() = default;

    virtual PlayerStateId GetStateId() const noexcept = 0;
    virtual void Enter(Player& player) = 0;
    virtual void Update(Player& player) = 0;
    virtual void Exit(Player& player) = 0;
};

