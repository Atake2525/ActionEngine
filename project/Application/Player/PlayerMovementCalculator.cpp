#include "PlayerMovementCalculator.h"

#include "kMath.h"

#include <algorithm>
#include <cmath>

Vector3 PlayerMovementCalculator::CalculateHorizontalVelocity(
    const Vector3& inputDirection,
    const Vector3& currentVelocity,
    float speed,
    float accelerationTime,
    float deltaTime)
{
    const float inputLength = Length(inputDirection);
    if (inputLength <= 0.0f)
    {
        return Vector3::Zero;
    }

    Vector3 adjustedInput = inputDirection;
    if (inputLength > 1.0f)
    {
        adjustedInput = Normalize(inputDirection);
    }

    const Vector3 targetVelocity = adjustedInput * speed;
    if (accelerationTime <= 0.0f || deltaTime <= 0.0f)
    {
        return targetVelocity;
    }

    const Vector3 difference = targetVelocity - currentVelocity;
    const float distance = Length(difference);
    const float acceleration = speed / accelerationTime;
    const float step = acceleration * deltaTime;

    if (distance <= step)
    {
        return targetVelocity;
    }

    return currentVelocity + Normalize(difference) * step;
}

float PlayerMovementCalculator::CalculateJumpVelocity(float gravityY, float jumpHeight)
{
    const float gravityMagnitude = std::max(-gravityY, 0.0f);
    const float validJumpHeight = std::max(jumpHeight, 0.0f);
    return std::sqrt(2.0f * gravityMagnitude * validJumpHeight);
}

float PlayerMovementCalculator::CalculateFallVelocity(
    float currentVelocity,
    float gravityY,
    float groundDistance,
    float deltaTime)
{
    if (deltaTime <= 0.0f)
    {
        return currentVelocity;
    }

    float nextVelocity = currentVelocity + gravityY * deltaTime;
    if (groundDistance >= 0.0f)
    {
        const float maximumFallVelocity = -groundDistance / deltaTime;
        if (nextVelocity < maximumFallVelocity)
        {
            nextVelocity = maximumFallVelocity;
        }
    }

    return nextVelocity;
}

Vector3 PlayerMovementCalculator::CalculateFrameMovement(
    const Vector3& horizontalVelocity,
    float verticalVelocity,
    float gravityY,
    bool applyGravity,
    float deltaTime)
{
    Vector3 movement = horizontalVelocity * deltaTime;
    movement.y = verticalVelocity * deltaTime;

    // 速度の積分はApplyGravityで行うため、ここでは同じフレームの加速度分だけを加える。
    // これによりジャンプ軌道がフレームレートで変化しにくくなる。
    if (applyGravity)
    {
        movement.y += 0.5f * gravityY * deltaTime * deltaTime;
    }

    return movement;
}
