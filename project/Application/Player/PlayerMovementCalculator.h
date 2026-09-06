#pragma once

#include "Vector3.h"

// プレイヤーの移動数値をワールド単位/秒で計算する。
// 減速補間は行わず、移動入力がない場合は水平速度を即座に0にする。
class PlayerMovementCalculator
{
public:
    static Vector3 CalculateHorizontalVelocity(
        const Vector3& inputDirection,
        const Vector3& currentVelocity,
        float speed,
        float accelerationTime,
        float deltaTime);

    static float CalculateJumpVelocity(float gravityY, float jumpHeight);

    static float CalculateFallVelocity(
        float currentVelocity,
        float gravityY,
        float groundDistance,
        float deltaTime);

    static Vector3 CalculateFrameMovement(
        const Vector3& horizontalVelocity,
        float verticalVelocity,
        float gravityY,
        bool applyGravity,
        float deltaTime);
};
