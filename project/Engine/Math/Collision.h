#pragma once
#include "AABB.h"
#include "Sphere.h"
#include <algorithm>
#include "kMath.h"
#include "Capsule.h"

inline const bool CollisionAABB(const AABB& a, const AABB& b) {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
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

inline const bool& CollisionCapsuleAABB(const Capsule& capsule, const AABB& aabb) {
	// AABB から線分に最も近い点を取るため、
	// AABB の中心から線分に垂線を落とし、最近接点を取得
	Vector3 boxCenter = (aabb.min + aabb.max) * 0.5f;

	// 線分上で boxCenter に最も近い点
	Vector3 segDir = capsule.end - capsule.start;
	float t = Dot(boxCenter - capsule.start, segDir) / Dot(segDir, segDir);
	t = std::clamp(t, 0.0f, 1.0f);  // 線分内にクランプ
	Vector3	closestOnSegment = capsule.start + segDir * t;
	

	// その点を AABB の内部にクランプする
	Vector3 clampedPoint;
	clampedPoint.x = std::clamp(closestOnSegment.x, aabb.min.x, aabb.max.x);
	clampedPoint.y = std::clamp(closestOnSegment.y, aabb.min.y, aabb.max.y);
	clampedPoint.z = std::clamp(closestOnSegment.z, aabb.min.z, aabb.max.z);

	// 線分との距離を判定
	float distSq = LengthSquared(closestOnSegment - clampedPoint);
	return distSq <= (capsule.radius * capsule.radius);


}

