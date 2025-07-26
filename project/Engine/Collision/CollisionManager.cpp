//#include "CollisionManager.h"
//#include "kMath.h"
//
////CollisionManager* CollisionManager::instance = nullptr;
//
////CollisionManager* CollisionManager::GetInstance() {
////	if (instance == nullptr)
////	{
////		instance = new CollisionManager;
////	}
////	return instance;
////}
//
//void CollisionManager::Initialize() {
//
//}
//
//void CollisionManager::Update() {
//	
//}
//
////void CollisionManager::Finalize() {
////	delete instance;
////	instance = nullptr;
////}
//
//const bool& CollisionManager::CheckCollision(const AABB& a, const AABB& b) {
//	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) && 
//		(a.min.y <= b.max.y && a.max.y >= b.min.y) && 
//		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
//		return true;
//	}
//	return false;
//}
//
//const bool& CollisionManager::CheckCollision(const AABB& target1, const Sphere& target2) {
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
//
////const bool& CollisionManager::CheckCollision(const AABB& target1, const OBB& target2) {
////
////}