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

inline const bool& CollisionAABBSphere(const AABB& aabb, const Sphere& sphere) {
	// 最近接点を求める
	Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
	};
	// 最近接点と球の中心との距離を求める
	Vector3 d = closestPoint - sphere.center;
	float distance = Dot(d, d);

	// 距離が半径よりも小さければ衝突
	if (distance <= sphere.radius * sphere.radius)
	{
		return true;
	}
	return false;
}

inline const bool CollisionCapsuleAABB(const Capsule& capsule, const AABB& aabb) {
	Vector3 closestOnAABB;
	closestOnAABB.x = std::clamp(capsule.start.x, aabb.min.x, aabb.max.x);
	closestOnAABB.y = std::clamp(capsule.start.y, aabb.min.y, aabb.max.y);
	closestOnAABB.z = std::clamp(capsule.start.z, aabb.min.z, aabb.max.z);

	// capsuleの線分の最近接点
	Vector3	closestOnCapsule;
	Vector3 ab = capsule.end - capsule.start;
	float abLen2 = Dot(ab, ab);

	Vector3 p = CenterAABB(aabb);
	float t = Dot(p - capsule.start, ab) / abLen2;

	closestOnCapsule = capsule.start * (1.0f - t) + capsule.end * t;

	// AABBとCapsuleの最近接点の距離を計算
	Vector3 dist = closestOnAABB - closestOnCapsule;
	float len = Dot(dist, dist);

	// 距離がcapsuleの半径よりも小さかったら衝突
	if (len <= capsule.radius * capsule.radius)
	{
		return true;
	}
	return  false;
}

inline const Vector3 CapsuleAABBPenetration(const Capsule& capsule, const AABB& aabb) {
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
	Vector3 dist = closestOnAABB - f;
	float distance = Dot(d, d);
	

	if (distance < capsule.radius * capsule.radius)
	{
		Vector3 dir = closestOnAABB - capsule.start;
		return (1.0f - dist) * capsule.radius;

	}
	return { 0.0f, 0.0f, 0.0f };
}

