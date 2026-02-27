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
	// aabbの最近接点
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

//inline const Vector3 CapsuleAABBPenetration(const Capsule& capsule, const AABB& aabb) {
//	// aabbの中心点を求める
//	// 1. AABB の中心ではなく、カプセル線分の start を AABB にクランプする
//	//    → これが AABB に最も近い点の候補になる
//	Vector3 p = capsule.start;
//	Vector3 q;
//
//	q.x = std::clamp(p.x, aabb.min.x, aabb.max.x);
//	q.y = std::clamp(p.y, aabb.min.y, aabb.max.y);
//	q.z = std::clamp(p.z, aabb.min.z, aabb.max.z);
//
//	Vector3 d = q - capsule.start;
//	Vector3 ba = capsule.end - capsule.start;
//	float t = Dot(d, ba) / Dot(ba, ba);
//	t = std::clamp(t, 0.0f, 1.0f);
//
//	// 求めたt(AABBの中心の最近接点)を使用してAABBの最近接点を求める
//
//	Vector3 f = capsule.start * (1.0f - t) + capsule.end * t;
//
//	// 4. ★ AABB に対して f をクランプ（これが重要）
//	Vector3 closestOnAABB;
//
//	closestOnAABB.x = std::clamp(f.x, aabb.min.x, aabb.max.x);
//	closestOnAABB.y = std::clamp(f.y, aabb.min.y, aabb.max.y);
//	closestOnAABB.z = std::clamp(f.z, aabb.min.z, aabb.max.z);
//
//	// 5. カプセル線分最近接点 f と AABB 最近接点の距離
//	Vector3 dist = closestOnAABB - f;
//	
//	return dist;
//
//
//	
//}

inline const Vector3 ColCapAABB(const Capsule& capsule, const AABB& aabb)
{
	// 1. AABB上の最近接点（カプセルの start を使うならそのまま）
	Vector3 closestOnAABB;
	closestOnAABB.x = std::clamp(capsule.start.x, aabb.min.x, aabb.max.x);
	closestOnAABB.y = std::clamp(capsule.start.y, aabb.min.y, aabb.max.y);
	closestOnAABB.z = std::clamp(capsule.start.z, aabb.min.z, aabb.max.z);

	// 2. capsule 中心線分上の最近接点（AABB中心に対して）
	Vector3 ab = capsule.end - capsule.start;
	float abLen2 = Dot(ab, ab);
	if (abLen2 <= 0.0f) {
		// 退化: カプセルが球と同じなので球 vs AABB として処理する
		Vector3 dist = closestOnAABB - capsule.start;
		float centerDist = std::sqrt(Dot(dist, dist));
		float penetration = capsule.radius - centerDist;
		if (penetration <= 0.0f)
			return Vector3(0.0f, 0.0f, 0.0f);

		Vector3 normal = (centerDist > 0.0f)
			? dist / centerDist
			: Vector3(0.0f, 1.0f, 0.0f); // 適当な方向
		return normal * penetration;
	}

	Vector3 p = CenterAABB(aabb);
	float t = Dot(p - capsule.start, ab) / abLen2;
	t = std::clamp(t, 0.0f, 1.0f);                       // 線分にクランプ
	Vector3 closestOnCapsule = capsule.start * (1.0f - t) + capsule.end * t;

	// 3. 中心線とAABBの距離
	Vector3 diff = closestOnAABB - closestOnCapsule;
	float centerDist2 = Dot(diff, diff);

	// 衝突していない
	float r = capsule.radius;
	if (centerDist2 > r * r)
		return Vector3(0.0f, 0.0f, 0.0f);

	float centerDist = std::sqrt(centerDist2);
	float penetration = r - centerDist;                  // 貫通量（スカラー）

	// 4. 法線（AABB → カプセル）
	Vector3 normal;
	if (centerDist > 0.0f) {
		normal = diff / centerDist;                      // normalize(diff)
	}
	else {
		// 完全にめり込み & 中心線が箱内にあるケース → 適当な軸
		normal = Vector3(0.0f, 1.0f, 0.0f);
	}

	// 5. 押し戻しベクトル = 法線 * 貫通量
	return normal * penetration;
}
