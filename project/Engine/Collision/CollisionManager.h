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

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 貫通量を取得
	/// </summary>
	const Vector3& GetPenetration() const { return m_penetration; }

    const Vector3 GetPenetrationForAABB(const AABB& aabb, bool wallDashCollision = false);

	/// <summary>
	/// ３方向全ての貫通量をreturn
	/// </summary>
	const Vector3 GetAllPenetrationForAABB(const AABB& aabb, bool wallDashCollision = false);

	// 対象と地面との距離を求める
	const float GetGroundDistance(const std::string& targetName, bool wallDashCollision = false) const;

	const float GetGroundDistanceForAABB(const AABB& aabb, bool wallDashCollision = false) const;

	// 衝突しているオブジェクトのAABBを変えす(複数のオブジェクトに衝突している可能性を考えてvectorで)
	const std::vector<AABB> GetCollisionObjectAABBsForAABB(const AABB& aabb, bool wallRunCollision = false) const;

	// 対象と地面との最大距離を求める
	const float GetGroundMAXDistance(const std::string& targetName, bool wallDashCollision = false) const;

    // 指定した座標と地面との最大距離を求める
    const float GetMaxGroundDistanceForAABB(const AABB& aabb, bool wallDashCollision = false) const;


	const Vector3 CheckPenetrationAmount(const AABB& aabb);

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
