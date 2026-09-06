#pragma once

#include "PlayerBaseState.h"

#include <array>
#include <cstddef>

// 1フレーム内で判断済みのプレイヤー状態。
// Playerの物理判定と、Stateの優先順位を分離するために使用する。
struct PlayerActionContext
{
    bool wasWallRunning = false;
    bool isWallRunning = false;
    bool isOnGround = false;
    bool wantsToCrouch = false;
    bool needsCrouchForClearance = false;
    bool jumpRequested = false;
    bool isClimbing = false;
};

// 同じフレームで複数のStateのEnter処理が必要になる場合があるため、
// 最終Stateだけでなく適用順も保持する。
class PlayerStateTransitionPlan
{
public:
    void Add(PlayerStateId stateId);

    std::size_t GetCount() const noexcept { return m_count; }
    PlayerStateId GetStateId(std::size_t index) const noexcept { return m_stateIds[index]; }

private:
    // 通常移動 + WallRun + Jump + WallJump + Climbing をすべて保持できる数。
    // 通常は地上とWallRunが同時成立しないが、不整合な入力でも優先Stateを欠落させない。
    static constexpr std::size_t kMaxTransitions = 5;

    std::array<PlayerStateId, kMaxTransitions> m_stateIds{};
    std::size_t m_count = 0;
};

// Stateの選択順を一か所に集約する。
// 後から追加されたStateほど優先度が高く、最終的なStateになる。
class PlayerActionResolver
{
public:
    static PlayerStateTransitionPlan Resolve(const PlayerActionContext& context);
};
