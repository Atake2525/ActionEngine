#include "Vector3.h"

#pragma once

struct AABB final
{
	Vector3 min;
	Vector3 max;
};

inline AABB operator+=(AABB& aabb, const float& value) {
	aabb.min += value;
	aabb.max += value;
	return aabb;
}

inline AABB operator+=(AABB& aabb1, const AABB& aabb2) {
	aabb1.min += aabb2.min;
	aabb1.max += aabb2.max;
	return aabb1;
}

inline AABB operator+=(AABB& aabb, const Vector3& value) {
	aabb.min += value;
	aabb.max += value;
	return aabb;
}

inline AABB AddSize(AABB& aabb, const float& value) {
	aabb.min -= value;
	aabb.max += value;
	return aabb;
}

inline AABB operator+(const AABB& aabb, const Vector3 value) {
	AABB result;
	result.min += aabb.min + value;
	result.max += aabb.max + value;
	return result;
}