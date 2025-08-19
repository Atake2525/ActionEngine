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

	void Update();

	void Finalize();

	const Vector3& GetPenetration() const { return penetration_; }

	// 当たり判定の追加
	//const bool& CheckCollision(const AABB& a, const AABB& b);
	//const bool& CheckCollision(const AABB& a, const Sphere& b);


	//const bool& CheckCollision(const AABB& a, const OBB& b);

	// 当たり判定として計算するオブジェクトの追加
	void AddCollision(Object3d* object3d, const std::string key);

	// 当たり判定を計算する対象の追加(念のため複数指定できるようにしておく)
	void AddCollisionTarget(Object3d* target, const std::string key);

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
