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

	// 当たり判定の追加
	//const bool& CheckCollision(const AABB& a, const AABB& b);
	//const bool& CheckCollision(const AABB& a, const Sphere& b);


	//const bool& CheckCollision(const AABB& a, const OBB& b);

	void AddCollisionTraget(Object3d* object3d, const std::string key);

private:
	std::map<std::string, Object3d*> collisionObject;
};
