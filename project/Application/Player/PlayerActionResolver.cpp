#include "PlayerActionResolver.h"

void PlayerStateTransitionPlan::Add(PlayerStateId stateId)
{
    if (m_count >= m_stateIds.size())
    {
        return;
    }

    m_stateIds[m_count] = stateId;
    m_count++;
}

PlayerStateTransitionPlan PlayerActionResolver::Resolve(const PlayerActionContext& context)
{
    PlayerStateTransitionPlan plan;

    // 通常移動State。壁走り開始・終了判定より前の状態を使い、既存の挙動を維持する。
    if (context.wantsToCrouch || context.needsCrouchForClearance)
    {
        plan.Add(PlayerStateId::Crouch);
    }
    else if (!context.wasWallRunning)
    {
        if (context.isOnGround)
        {
            plan.Add(PlayerStateId::Run);
        }
        else
        {
            plan.Add(PlayerStateId::AirControl);
        }
    }

    // パルクールState。下に書かれた条件ほど優先度が高い。
    if (context.isWallRunning)
    {
        plan.Add(PlayerStateId::WallRun);
    }

    if (context.jumpRequested)
    {
        if (context.isOnGround)
        {
            plan.Add(PlayerStateId::Jump);
        }

        if (context.isWallRunning)
        {
            plan.Add(PlayerStateId::WallJump);
        }

        if (context.isClimbing)
        {
            plan.Add(PlayerStateId::Climbing);
        }
    }

    return plan;
}
