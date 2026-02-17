#pragma once
#include "kMath.h"
#include "AABB.h"
#include "Plane.h"
#include "Sphere.h"
#include "OBB.h"
#include <algorithm>
#include "Object3d.h"

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
private:
	// シングルトンパターンの適用
	static CollisionManager* instance;

	// コンストラクタ、デストラクタの隠蔽
	CollisionManager() = default;
	~CollisionManager() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	CollisionManager(CollisionManager&) = delete;
	CollisionManager& operator=(CollisionManager&) = delete;

public:

	// インスタンスの取得
	static CollisionManager* GetInstance();

	/// <summary>
	/// 取得
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const std::string& targetName, bool wallDashCollision = false);

	// 対象(Capsule)と障害物(AABB)の衝突量を求める
	//void UpdateCupsulCollision(const std::string& targetName);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 貫通量を取得
	/// </summary>
	const Vector3& GetPenetration() const { return m_penetration; }

    Vector3 GetPenetrationForAABB(const AABB& aabb, bool wallDashCollision = false);

	// 対象と地面との距離を求める
	const float GetGroundDistance(const std::string& targetName, bool wallDashCollision = false) const;

	// 対象と地面との最大距離を求める
	const float GetGroundMAXDistance(const std::string& targetName, bool wallDashCollision = false) const;

    // 指定した座標と地面との最大距離を求める
    const float GetMaxGroundDistanceForAABB(const AABB& aabb, bool wallDashCollision = false) const;

	// 当たり判定の追加
	//const bool& CheckCollision(const AABB& a, const AABB& b);
	//const bool& CheckCollision(const AABB& a, const Sphere& b);

	const Vector3 CheckPenetrationAmount(const AABB& aabb);

	//const bool& CheckCollision(const AABB& a, const OBB& b);

	// 当たり判定として計算するオブジェクトの追加
	void AddCollision(Object3d* object3d);

	void AddWallDashColliison(Object3d* object3d);

	// 当たり判定を計算する対象の追加(念のため複数指定できるようにしておく)
	void AddCollisionTarget(AABB aabb, const std::string key);

	void UpdateCollisionTarget(AABB aabb, const std::string key);

	// 判定対象オブジェクトの削除
	void DeleteCollision(Object3d* object3d);

	// 判定対象オブジェクトの削除
	void DeleteWallDashCollision(Object3d* object3d);

	// 判定対象の削除
	void DeleteCollisionTarget(const std::string key);

	//private:
		// 貫通量の計算(当たり判定)横
	const Vector3 GetPenetrationDepth(const AABB& target, const AABB& object);

	CollisionDirection GetVerticalCollisionDirection(const AABB& target, const AABB& object);


private:

	std::vector<Object3d*> collisionObject;

	std::vector<Object3d*> wallDashCollisionObject;

	std::map<std::string, AABB> collisionTarget;

	// 貫通量を示す変数
	Vector3 m_penetration;
	Vector3 m_penetrationPre;

	// 衝突しているオブジェクトのAABB
	std::vector<AABB> m_collisionObjectAABB;

};
