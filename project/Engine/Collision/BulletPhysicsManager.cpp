//#include "BulletPhysicsManager.h"
//#include "kMath.h"
//
//void BulletPhysicsManager::SetGround(Object3d* object3d)
//{
//    std::vector<AABB> objectAABBs = object3d->GetAABBMultiMeshed();
//
//	for (AABB objectAABB : objectAABBs)
//	{
//		Vector3 center = CenterAABB(objectAABB);
//		Vector3 halfSize = AABB::GetSize(objectAABB);
//		SetGroundShape(halfSize, center);
//	}
//}
//
//void BulletPhysicsManager::SetPlayerShape(Vector3 pos, float radius, float height, float mass)
//{
//	btCollisionShape* shape = new btCapsuleShape(radius, height);
//	shapes.push_back(shape);
//
//	btTransform t;
//	t.setIdentity();
//	t.setOrigin(btVector3{ pos.x, pos.y, pos.z });
//
//	btVector3 inertia(0, 0, 0);
//	shape->calculateLocalInertia(mass, inertia);
//
//	btDefaultMotionState* motion = new btDefaultMotionState(t);
//
//	btRigidBody::btRigidBodyConstructionInfo info(
//		mass, motion, shape, inertia
//	);
//
//	btRigidBody* body = new btRigidBody(info);
//
//	// 高速移動でも貫通しないようにCCDを有効化
//	body->setCcdMotionThreshold(0.01f);
//	body->setCcdSweptSphereRadius(radius * 0.8f);
//
//	world->addRigidBody(body);
//	bodies.push_back(body);
//}
//
//void BulletPhysicsManager::Step(float delta)
//{
//	world->stepSimulation(delta, 1, delta);
//}
//
//void BulletPhysicsManager::SetGroundShape(const Vector3& halfExtents, const Vector3& pos)
//{
//	btVector3 half = { halfExtents.x, halfExtents.y, halfExtents.z };
//	btCollisionShape* shape = new btBoxShape(half);
//	shapes.push_back(shape);
//
//	btTransform t;
//	t.setIdentity();
//	t.setOrigin(btVector3{ pos.x, pos.y, pos.z });
//
//	btDefaultMotionState* motion = new btDefaultMotionState(t);
//
//	btRigidBody::btRigidBodyConstructionInfo info(
//		0.0f, motion, shape
//	);
//
//	btRigidBody* body = new btRigidBody(info);
//	world->addRigidBody(body);
//	bodies.push_back(body);
//
//}
