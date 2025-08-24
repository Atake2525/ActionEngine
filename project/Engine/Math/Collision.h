#pragma once
#include "AABB.h"
#include "Sphere.h"
#include <algorithm>
#include "kMath.h"
#include "Capsule.h"

inline const bool CollisionAABB(const AABB& a, const AABB& b) {
	if ((a.min.x < b.max.x && a.max.x > b.min.x) &&
		(a.min.y < b.max.y && a.max.y > b.min.y) &&
		(a.min.z < b.max.z && a.max.z > b.min.z)) {
		return true;
	}
	return false;
}

inline const bool CollisionAABBXZ(const AABB& a, const AABB& b) {
	if ((a.min.x < b.max.x && a.max.x > b.min.x) &&
		(a.min.z < b.max.z && a.max.z > b.min.z)) {
		return true;
	}
	return false;
}

inline const bool& CollisionAABBSphere(const AABB& target1, const Sphere& target2) {
	// 最近接点を求める
	Vector3 closestPoint{
		std::clamp(target2.center.x, target1.min.x, target1.max.x),
		std::clamp(target2.center.y, target1.min.y, target1.max.y),
		std::clamp(target2.center.z, target1.min.z, target1.max.z)
	};
	// 最近接点と球の中心との距離を求める
	float distance = Length(closestPoint - target2.center);
	// 距離が半径よりも小さければ衝突
	if (distance <= target2.radius)
	{
		return true;
	}
	return false;
}

inline const bool CollisionCapsuleAABB(const Capsule& capsule, const AABB& aabb) {
    // 線分方向と長さ
    Vector3 segDir = capsule.end - capsule.start;
    float segLengthSq = Dot(segDir, segDir);

    // 線分が退化している場合（start == end）
    if (segLengthSq == 0.0f) {
        // 点と AABB の最近接点を比較
        Vector3 clamped = {
            std::clamp(capsule.start.x, aabb.min.x, aabb.max.x),
            std::clamp(capsule.start.y, aabb.min.y, aabb.max.y),
            std::clamp(capsule.start.z, aabb.min.z, aabb.max.z)
        };
        float distSq = LengthSquared(capsule.start - clamped);
        return distSq <= capsule.radius * capsule.radius;
    }

    // 線分上で AABB に最も近い点を求める
    // AABB の中心から線分への垂線を使って近似
    Vector3 boxCenter = (aabb.min + aabb.max) * 0.5f;
    float t = Dot(boxCenter - capsule.start, segDir) / segLengthSq;
    t = std::clamp(t, 0.0f, 1.0f);
    Vector3 closestOnSegment = capsule.start + segDir * t;

    // AABB 内にクランプ
    Vector3 clampedPoint = {
        std::clamp(closestOnSegment.x, aabb.min.x, aabb.max.x),
        std::clamp(closestOnSegment.y, aabb.min.y, aabb.max.y),
        std::clamp(closestOnSegment.z, aabb.min.z, aabb.max.z)
    };

    // 距離判定
    float distSq = LengthSquared(closestOnSegment - clampedPoint);
    return distSq <= capsule.radius * capsule.radius;
}

