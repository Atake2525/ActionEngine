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
	penetration_ = { 0.0f, 0.0f, 0.0f };
	collisionTarget.clear();
	collisionObject.clear();
}

void CollisionManager::Update(const std::string& targetName) {
	//auto target = collisionTarget.find(targetName);

	//if (target != collisionTarget.end()) {
	//	Object3d* obj = target->second;
	//	// obj を使って処理
	//}
	//else {
	//	// キーが存在しない場合の処理
	//}

	penetration_ = { 0.0f, 0.0f, 0.0f };
	for (const auto& object : collisionObject) {
		// ターゲット(プレイヤーなど)とオブジェクトの距離を全体のAABBから求めて離れていればcontinue
		float dist = Distance(CenterAABB(collisionTarget[targetName]->GetAABB()), CenterAABB(object.second->GetAABB()));
		// オブジェクトの大きさを求める
		AABB objectAABB = object.second->GetAABB();
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
				if (CollisionAABB(collisionTarget[targetName]->GetAABB(), terrainAABB))
				{
					Vector3 penetration = GetPenetrationDepth(collisionTarget[targetName]->GetAABB(), terrainAABB);
					float minDepth = std::min(penetration.x, std::min(penetration.z, penetration.y));
					if (minDepth == penetration.x)
					{
						penetration.y = 0.0f;
						penetration.z = 0.0f;
					}
					else if (penetration.z == minDepth)
					{
						penetration.x = 0.0f;
						penetration.y = 0.0f;
					}
					else if (penetration.y == minDepth)
					{
						penetration.x = 0.0f;
						penetration.z = 0.0f;
					}

					// 方向に応じて押し出す方向が変わるので確認する
					Vector3 center = CenterAABB(terrainAABB);
					if (collisionTarget[targetName]->GetTranslate().x > center.x)
					{
						penetration.x *= -1.0f;
					}
					if (collisionTarget[targetName]->GetTranslate().z > center.z)
					{
						penetration.z *= -1.0f;
					}
					if (collisionTarget[targetName]->GetTranslate().y > center.y)
					{
						penetration.y *= -1.0f;
					}
						
					// 押し出しの量を格納する
					penetration_ += penetration;
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

void CollisionManager::Finalize() {
	collisionObject.clear();
	delete instance;
	instance = nullptr;
}

const float CollisionManager::GetGroundDistance(const std::string& targetName) const {
	auto target = collisionTarget.find(targetName);

	if (target != collisionTarget.end()) {
		// obj を使って処理
	}
	else {
		// キーが存在しない場合の処理
	}

	float distance = 100.0f;
	for (const auto& object : collisionObject) {
		// オブジェクトのメッシュごとのAABBを取得する
		float serchDistance = Distance(object.second->GetAABB().max, target->second->GetAABB().min);
		if (serchDistance <= distance)
		{
			const std::vector<AABB> terrains = object.second->GetAABBMultiMeshed();
			for (AABB terrainAABB : terrains)
			{
				terrainAABB = AddSize(terrainAABB, 0.1f);
				// ターゲットとオブジェクトが貫通していたら実行
				if (CollisionAABBXZ(target->second->GetAABB(), terrainAABB))
				{
					float dist = target->second->GetAABB().min.y - terrainAABB.max.y;
					distance = std::min(distance, dist);
				}
			}
		}
	}
	return distance;
}

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
	result.y = std::min(object.max.y, target.max.y) - max(object.min.y, target.min.y);
	result.z = std::min(object.max.z, target.max.z) - max(object.min.z, target.min.z);

	// 貫通していない場合は0にする
	if (result.x < 0) { result.x = 0; }
	if (result.y < 0) { result.y = 0; }
	if (result.z < 0) { result.z = 0; }

	return result;
}

CollisionDirection CollisionManager::GetVerticalCollisionDirection(const AABB& target, const AABB& object)
{
	if (object.max.y > target.min.y && object.min.y < target.min.y) {
		return CollisionDirection::FromBelow;
	}
	else if (object.min.y < target.max.y && object.max.y > target.max.y) {
		return CollisionDirection::FromAbove;
	}
	return CollisionDirection::None;
}