//#include "externals/BulletPhysics/src/btBulletCollisionCommon.h"
//#include "externals/BulletPhysics/src/btBulletDynamicsCommon.h"
//#include <vector>
//#include <iostream>
//#include "Vector3.h"
//#include "Object3d.h"
//
//#pragma once
//
//class BulletPhysicsManager
//{
//public:
//    
//    void SetGround(Object3d* object3d);
//    void SetPlayerShape(Vector3 pos, float radius, float height, float mass = 1.0f);
//    void Step(float delta);
//
//private:
//    void SetGroundShape(const Vector3& halfExtents, const Vector3& pos);
//    std::vector<btCollisionShape*> shapes;
//    std::vector<btRigidBody*> bodies;
//
//    btDefaultCollisionConfiguration* config;
//    btCollisionDispatcher* dispatcher;
//    btBroadphaseInterface* broadphase;
//    btSequentialImpulseConstraintSolver* solver;
//    btDiscreteDynamicsWorld* world;
//
//};
//
