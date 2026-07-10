#pragma once

class Player;

// 移動タイプ
enum class PlayerStateId {
    Run,
    Crouch,
    WallRun,
    WallJump,
    Climbing
};

class PlayerState {
public:
    virtual ~PlayerState() = default;

    virtual PlayerStateId GetStateId() const noexcept = 0;
    virtual void Enter(Player& player) = 0;
    virtual void Update(Player& player) = 0;
    virtual void Exit(Player& player) = 0;
};

