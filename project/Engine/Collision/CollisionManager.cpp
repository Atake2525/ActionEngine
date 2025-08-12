#include "CollisionManager.h"
#include "Collision.h"
#include "kMath.h"
#include "Logger.h"

#define NOMINMAX

using namespace Logger;

CollisionManager* CollisionManager::instance = nullptr;

CollisionManager* CollisionManager::GetInstance() {
	if (instance == nullptr)
	{
		instance = new CollisionManager;
	}
	return instance;
}

void CollisionManager::Initialize() {
	penetration = { 0.0f, 0.0f, 0.0f };
	collisionTarget.clear();
	collisionObject.clear();
}

void CollisionManager::Update() {
	penetration = { 0.0f, 0.0f, 0.0f };
	for (const auto& target : collisionTarget) {
		for (const auto& object : collisionObject) {
			// ターゲット(プレイヤーなど)とオブジェクトの距離を全体のAABBから求めて離れていればcontinue
			float dist = Distance(CenterAABB(target.second->GetAABB()), CenterAABB(object.second->GetAABB()));
			// オブジェクトの大きさを求める
			AABB objectAABB = object.second->GetAABB();
			// ターゲットとの最近接点を取る
			//Vector3 closestPoint = ClosestPoint(objectAABB, target.second->GetTranslate());
			// 最近接点とオブジェクトの中心座標の距離を取ってプレイヤーからオブジェクトまでの直線の距離を求める
			float objectSize = Distance(objectAABB.min, objectAABB.max);
			// オブジェクトサイズよりも距離が近かったら処理をする(余裕をもって少しだけ広く)
			if (dist < objectSize + 0.0f)
			{
				// オブジェクトのメッシュごとのAABBを取得する
				const std::vector<AABB> terrains = object.second->GetAABBMultiMeshed();
				for (AABB terrainAABB : terrains)
				{
					terrainAABB = AddSize(terrainAABB, 0.1f);
					// ターゲットとオブジェクトが貫通していたら実行
					if (CollisionAABB(target.second->GetAABB(), terrainAABB))
					{
						penetration = GetPenetrationDepth(target.second->GetAABB(), terrainAABB);
						float minDepth = std::min(penetration.x, penetration.z);
						if (minDepth == penetration.x)
						{
							//penetration.x += 1.0f;
							penetration.z = 0.0f;
						}
						else if (penetration.z == minDepth)
						{
							//penetration.z += 1.0f;
							penetration.x = 0.0f;
						}
						Vector3 center = CenterAABB(terrainAABB);
						if (target.second->GetTranslate().x > center.x)
						{
							penetration.x *= -1.0f;
						}
						if (target.second->GetTranslate().z > center.z)
						{
							penetration.z *= -1.0f;
						}
						break;
					}
				}
			}
			else
			{
				// 離れていればcontinue
				continue;
			}
		}
	}
}

void CollisionManager::Finalize() {
	collisionObject.clear();
	delete instance;
	instance = nullptr;
}

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

//const bool& CollisionManager::CheckCollision(const AABB& target1, const OBB& target2) {
//
//}

// 障害物の追加
void CollisionManager::AddCollision(Object3d* object3d, const std::string key) {
	if (!collisionObject.contains(key))
	{
		collisionObject[key] = object3d;
	}
	else
	{
		Log("既に登録されているキーが指定されています\n実行 : AddCollision\n CollisionManager.cpp\n");
	}
}

// 衝突対象の追加
void CollisionManager::AddCollisionTarget(Object3d* object3d, const std::string key)
{
	if (!collisionTarget.contains(key))
	{
		collisionTarget[key] = object3d;
	}
	else
	{
		Log("既に登録されているキーが指定されています\n実行 : AddCollisionTarget コード : CollisionManager.cpp\n");
	}
}

const Vector3 CollisionManager::GetPenetrationDepth(const AABB& target, const AABB& object)
{
	Vector3 result = { 0.0f, 0.0f, 0.0f };

	result.x = std::min(object.max.x, target.max.x) - max(object.min.x, target.min.x);
	//result.y = std::min(object.max.y, target.max.y) - max(object.min.y, object.min.y);
	result.z = std::min(object.max.z, target.max.z) - max(object.min.z, target.min.z);

	// 貫通していない場合は0にする
	if (result.x < 0) { result.x = 0; }
	if (result.y < 0) { result.y = 0; }
	if (result.z < 0) { result.z = 0; }


	return result;
}
