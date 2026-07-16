#pragma once
#include "kMath.h"
#include "AABB.h"
#include "Plane.h"
#include "Sphere.h"
#include "OBB.h"
#include <algorithm>
#include "Object3d.h"
#include <unordered_map>

enum class CollisionType {
	Sphere,
	AABB,
	OBB,
	Plat,
};

enum class CollisionDirection {
	None,
	FromAbove,  // プレイヤーが下から上にぶつかった（天井）
	FromBelow   // プレイヤーが上から下にぶつかった（地面）
};


// 当たり判定マネージャー
class CollisionManager {
public:
	CollisionManager();
	~CollisionManager();

	/// <summary>
	/// 取得
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const std::string& targetName);

	/// <summary>
	/// 貫通量を取得
	/// </summary>
	const Vector3& GetPenetration() const { return m_penetration; }

    const Vector3 GetPenetrationForAABB(const AABB& aabb);

	/// <summary>
	/// ３方向全ての貫通量をreturn
	/// </summary>
	const Vector3 GetAllPenetrationForAABB(const AABB& aabb);

	// 対象と地面との距離を求める
	const float GetGroundDistance(const std::string& targetName) const;

	const float GetGroundDistanceForAABB(const AABB& aabb) const;

	// 衝突しているオブジェクトのAABBを変えす(複数のオブジェクトに衝突している可能性を考えてvectorで)
	const std::vector<AABB> GetCollisionObjectAABBsForAABB(const AABB& aabb) const;

	// 対象と地面との最大距離を求める
	const float GetGroundMAXDistance(const std::string& targetName) const;

    // 指定した座標と地面との最大距離を求める
    const float GetMaxGroundDistanceForAABB(const AABB& aabb) const;

	//const bool IsCollisionObjectForAABB(const AABB& aabb, bool wallDashCollision, const AABB& noSearchAABB = AABB::Zero) const;

	const float GetHeightToTopForAABB(const AABB& aabb);

	const Vector3 GetCollisionObjectDirectionForAABB(const AABB& aabb);

	const AABB GetObjectForCollisionDirection(const AABB& aabb, const Vector3& direction);
	const bool IsCollisionObjectForAABB(const AABB& aabb, const AABB& noSearchAABB = AABB::Zero) const;

	const Vector3 CheckPenetrationAmount(const AABB& aabb);

	// 当たり判定として計算するオブジェクトの追加
	void AddCollision(Object3d* object3d);

	// 当たり判定を計算する対象の追加(念のため複数指定できるようにしておく)
	void AddCollisionTarget(AABB aabb, const std::string key);

	void UpdateCollisionTarget(AABB aabb, const std::string key);

	// 判定対象オブジェクトの削除
	void DeleteCollision(Object3d* object3d);

	// 判定対象の削除
	void DeleteCollisionTarget(const std::string key);

	//private:
		// 貫通量の計算(当たり判定)横
	const Vector3 GetPenetrationDepth(const AABB& target, const AABB& object);

	CollisionDirection GetVerticalCollisionDirection(const AABB& target, const AABB& object);


private:

	std::vector<Object3d*> collisionObject;

	std::unordered_map<std::string, AABB> collisionTarget;

	// 貫通量を示す変数
	Vector3 m_penetration;
	Vector3 m_penetrationPre;

	// 衝突しているオブジェクトのAABB
	std::vector<AABB> m_collisionObjectAABB;

};
