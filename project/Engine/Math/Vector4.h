#include "Vector3.h"
#pragma once

struct Vector4
{
	float x;
	float y;
	float z;
	float w;

};

inline Vector4 operator*(const Vector4& v1, const Vector3& v2) {
	return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w };
}