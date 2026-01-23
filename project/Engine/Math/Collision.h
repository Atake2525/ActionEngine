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
	// aabbの中心点を求める
	// 1. AABB の中心ではなく、カプセル線分の start を AABB にクランプする
	//    → これが AABB に最も近い点の候補になる
	Vector3 p = capsule.start;
	Vector3 q;

	q.x = std::clamp(p.x, aabb.min.x, aabb.max.x);
	q.y = std::clamp(p.y, aabb.min.y, aabb.max.y);
	q.z = std::clamp(p.z, aabb.min.z, aabb.max.z);

	Vector3 d = q - capsule.start;
	Vector3 ba = capsule.end - capsule.start;
	float t = Dot(d, ba) / Dot(ba, ba);
	t = std::clamp(t, 0.0f, 1.0f);

	// 求めたt(AABBの中心の最近接点)を使用してAABBの最近接点を求める

	Vector3 f = capsule.start * (1.0f - t) + capsule.end * t;

	// 4. ★ AABB に対して f をクランプ（これが重要）
	Vector3 closestOnAABB;

	closestOnAABB.x = std::clamp(f.x, aabb.min.x, aabb.max.x);
	closestOnAABB.y = std::clamp(f.y, aabb.min.y, aabb.max.y);
	closestOnAABB.z = std::clamp(f.z, aabb.min.z, aabb.max.z);

	// 5. カプセル線分最近接点 f と AABB 最近接点の距離
	float distance = Length(closestOnAABB - f);


	if (distance < capsule.radius)
	{
		return true;
	}
	return false;
}

