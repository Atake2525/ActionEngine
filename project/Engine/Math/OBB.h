#include "Vector3.h"
#include "Transform.h"
#include "kMath.h"

#pragma once

struct OBB
{
    Vector3 center;       // 中心座標（ワールド空間）
    Vector3 axes[3];      // 各軸（ワールド空間での方向ベクトル）
    Vector3 halfSize;     // 各軸方向の半分の長さ（スケール）
};

inline const OBB CreateOBB(const Matrix4x4 affine, const Vector3 halfSize) {
    
    OBB obb;
    obb.center = { affine.m[3][0], affine.m[3][1], affine.m[3][2] };

    obb.axes[0] = Normalize({ affine.m[0][0], affine.m[0][1], affine.m[0][2] }); // X軸
    obb.axes[1] = Normalize({ affine.m[1][0], affine.m[1][1], affine.m[1][2] }); // Y軸
    obb.axes[2] = Normalize({ affine.m[2][0], affine.m[2][1], affine.m[2][2] }); // Z軸

    obb.halfSize = halfSize;
    return obb;
}

inline const OBB CreateOBB(const Transform transform, const AABB aabb) {
    OBB obb;
    Matrix4x4 matrix = MakeAffineMatrix(transform);

    obb.axes[0] = Normalize({ matrix.m[0][0], matrix.m[0][1], matrix.m[0][2] });
    obb.axes[1] = Normalize({ matrix.m[1][0], matrix.m[1][1], matrix.m[1][2] });
    obb.axes[2] = Normalize({ matrix.m[2][0], matrix.m[2][1], matrix.m[2][2] });

    obb.center = (aabb.min + aabb.max) * 0.5f;
    obb.halfSize = (aabb.max - aabb.min) * 0.5f;

    return obb;
}

inline const OBB CreateOBB(const Matrix4x4 affine, const Vector3 halfSize, const Vector3 center) {

    OBB obb;
    obb.center = center;

    obb.axes[0] = Normalize({ affine.m[0][0], affine.m[0][1], affine.m[0][2] }); // X軸
    obb.axes[1] = Normalize({ affine.m[1][0], affine.m[1][1], affine.m[1][2] }); // Y軸
    obb.axes[2] = Normalize({ affine.m[2][0], affine.m[2][1], affine.m[2][2] }); // Z軸

    obb.halfSize = halfSize;
    return obb;
}

inline float ProjectOBB(const OBB& obb, const Vector3& axis) {
    return std::abs(Dot(obb.axes[0], axis)) * obb.halfSize.x +
        std::abs(Dot(obb.axes[1], axis)) * obb.halfSize.y +
        std::abs(Dot(obb.axes[2], axis)) * obb.halfSize.z;
}


inline bool CheckOBBCollision(const OBB& a, const OBB& b) {
    const float EPSILON = 1e-6f;
    Vector3 axes[15];
    int axisCount = 0;

    // 1. 各OBBの軸
    for (int i = 0; i < 3; ++i) {
        axes[axisCount++] = a.axes[i];
        axes[axisCount++] = b.axes[i];
    }

    // 2. 外積による軸（A×B）
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vector3 cross = Cross(a.axes[i], b.axes[j]);
            if (Length(cross) > EPSILON) {
                axes[axisCount++] = Normalize(cross);
            }
        }
    }

    // 3. 各軸で投影して分離判定
    for (int i = 0; i < axisCount; ++i) {
        Vector3 axis = axes[i];

        float aProj = ProjectOBB(a, axis);
        float bProj = ProjectOBB(b, axis);
        float distance = std::abs(Dot(b.center - a.center, axis));

        if (distance > aProj + bProj) {
            return false; // 分離軸が存在 → 非衝突
        }
    }

    return true; // 全軸で重なりあり → 衝突
}
