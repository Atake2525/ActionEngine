#pragma once

#include <algorithm>

struct Vector3
{
	float x;
	float y;
	float z;

	// 減算演算子のオーバーロード
	Vector3 operator-(const Vector3& other) const;
};

inline Vector3 operator*=(Vector3& v, const float& n) {
	v.x *= n;
	v.y *= n;
	v.z *= n;
	return v;
}

inline Vector3 operator-=(Vector3& v, const float& n) {
	v.x -= n;
	v.y -= n;
	v.z -= n;
	return v;
}

inline Vector3 Vector3Clamp(Vector3& v, const float min, const float max) {
	v.x = std::clamp(v.x, min, max);
	v.y = std::clamp(v.y, min, max);
	v.z = std::clamp(v.z, min, max);
	
	return v;
}

inline Vector3 Vector3Clamp(Vector3& v, const Vector3 min, const float max) {
	v.x = std::clamp(v.x, min.x, max);
	v.y = std::clamp(v.y, min.y, max);
	v.z = std::clamp(v.z, min.z, max);

	return v;
}

inline Vector3 Vector3Clamp(Vector3& v, const float min, const Vector3 max) {
	v.x = std::clamp(v.x, min, max.x);
	v.y = std::clamp(v.y, min, max.y);
	v.z = std::clamp(v.z, min, max.z);

	return v;
}