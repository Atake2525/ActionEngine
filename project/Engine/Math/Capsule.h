#pragma once

#include "Vector3.h"
#include "kMath.h"

struct Capsule {
    Vector3 start;   // 始点（底側の球の中心）
    Vector3 end;     // 終点（上側の球の中心）
    float radius;    // 両端の球、および円柱部分の半径
};
