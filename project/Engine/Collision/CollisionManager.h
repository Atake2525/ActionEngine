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

	static CollisionManager* GetInstance();

	void Initialize();

	void Update(const std::string& targetName);

	// 対象(Capsule)と障害物(AABB)の衝突量を求める
	void UpdateCupsulCollision(const std::string& targetName);


	void Finalize();

	const Vector3& GetPenetration() const { return penetration_; }

	// 対象と地面との距離を求める
	const float GetGroundDistance(const std::string& targetName) const;

	// 当たり判定の追加
	//const bool& CheckCollision(const AABB& a, const AABB& b);
	//const bool& CheckCollision(const AABB& a, const Sphere& b);


	//const bool& CheckCollision(const AABB& a, const OBB& b);

	// 当たり判定として計算するオブジェクトの追加
	void AddCollision(Object3d* object3d, const std::string key);

	// 当たり判定を計算する対象の追加(念のため複数指定できるようにしておく)
	void AddCollisionTarget(Object3d* target, const std::string key);

	// 判定対象オブジェクトの削除
	void DeleteCollision(const std::string key);

	// 判定対象の削除
	void DeleteCollisionTarget(const std::string key);

//private:
	// 貫通量の計算(当たり判定)横
	const Vector3 GetPenetrationDepth(const AABB& target, const AABB& object);

	CollisionDirection GetVerticalCollisionDirection(const AABB& target, const AABB& object);


private:

	std::map<std::string, Object3d*> collisionObject;

	std::map<std::string, Object3d*> collisionTarget;

	// 貫通量を示す変数
	Vector3 penetration_;
};
