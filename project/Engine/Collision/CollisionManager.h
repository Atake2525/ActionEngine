#pragma once
#include "kMath.h"
#include "AABB.h"
#include "Plane.h"
#include "Sphere.h"
#include "OBB.h"
#include <algorithm>


//enum class CollisionType {
//	Sphere,
//	AABB,
//	OBB,
//	Plat,
//};


//class CollisionManager {
////private:
////	// シングルトンパターンの適用
////	static CollisionManager* instance;
////
////	// コンストラクタ、デストラクタの隠蔽
////	CollisionManager() = default;
////	~CollisionManager() = default;
////	// コピーコンストラクタ、コピー代入演算子の封印
////	CollisionManager(CollisionManager&) = delete;
////	CollisionManager& operator=(CollisionManager&) = delete;
//
//public:
//
//	//static CollisionManager* GetInstance();
//
//	void Initialize();
//
//	void Update();
//
//	//void Finalize();
//
//	// 当たり判定の追加
//	const bool& CheckCollision(const AABB& a, const AABB& b);
//	const bool& CheckCollision(const AABB& a, const Sphere& b);
//	//const bool& CheckCollision(const AABB& a, const OBB& b);
//};
//
const bool CollisionAABB(const AABB& a, const AABB& b) {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true;
	}
	return false;
}

//const bool& CollisionAABBSphere(const AABB& target1, const Sphere& target2) {
//	// 最近接点を求める
//	Vector3 closestPoint{
//		std::clamp(target2.center.x, target1.min.x, target1.max.x),
//		std::clamp(target2.center.y, target1.min.y, target1.max.y),
//		std::clamp(target2.center.z, target1.min.z, target1.max.z)
//	};
//	// 最近接点と球の中心との距離を求める
//	float distance = Length(closestPoint - target2.center);
//	// 距離が半径よりも小さければ衝突
//	if (distance <= target2.radius)
//	{
//		return true;
//	}
//	return false;
//}

