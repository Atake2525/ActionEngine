#include "Vector3.h"

#pragma once

struct AABB final
{
	Vector3 min;
	Vector3 max;

	static const Vector3 GetSize(const AABB& aabb);
	static const AABB MakeAABB(const Vector3& center, const Vector3& size);
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

inline AABB operator-=(AABB& aabb, const float& value) {
	aabb.min -= value;
	aabb.max -= value;
	return aabb;
}

inline AABB operator-=(AABB& aabb1, const AABB& aabb2) {
	aabb1.min -= aabb2.min;
	aabb1.max -= aabb2.max;
	return aabb1;
}

inline AABB operator-=(AABB& aabb, const Vector3& value) {
	aabb.min -= value;
	aabb.max -= value;
	return aabb;
}

inline AABB AddSize(AABB& aabb, const float& value) {
	/*aabb.min -= value;
	aabb.max += value;*/
	return aabb;
}

inline AABB operator+(const AABB& aabb, const Vector3 value) {
	AABB result;
	result.min = aabb.min + value;
	result.max = aabb.max + value;
	return result;
}

//inline AABB operator*(const AABB& aabb, const Vector3 value) {
//	AABB result;
//	result.min = aabb.min * value;
//	result.max = aabb.max * value;
//	return result;
//}

inline const Vector3 AABB::GetSize(const AABB& aabb) {
	return { aabb.max - aabb.min };
}

inline const AABB AABB::MakeAABB(const Vector3& center, const Vector3& size) {
	return {
		.min{center.x - size.x * 0.5f, center.y - size.y * 0.5f, center.z - size.z * 0.5f},
		.max{center.x + size.x * 0.5f, center.y + size.y * 0.5f, center.z + size.z * 0.5f}
	};
}