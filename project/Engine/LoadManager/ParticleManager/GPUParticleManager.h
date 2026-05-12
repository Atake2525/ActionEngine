#include "Transform.h"

struct ParticleCS
{
    Vector3 transalte;
    Vector3 scale;
    float lifeTime;
    Vector3 velocity;
    float currentTime;
    Vector4 color;
};

struct PreView
{
    Matrix4x4 viewProjection;
    Matrix4x4 billboardMatrix;
};

#pragma once
class GPUParticleManager
{

};

