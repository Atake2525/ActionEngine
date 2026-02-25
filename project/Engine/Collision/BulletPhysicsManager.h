#include "externals/BulletPhysics/src/btBulletCollisionCommon.h"
#include <vector>
#include <iostream>
#include "Vector3.h"

#pragma once

class BulletPhysicsManager
{
public:
    void SetUpGroundShape(const Vector3& halfEtents, const Vector3& pos);
    void SetPlayerShape(Vector3 trnslate);
    void Update();

private:
    std::vector<btCollisionShape*> groundShapes;
    std::vector<btRigidBody> groundBodies;
    btCollisionShape* playerShape;

    btDiscrete
};

