#pragma once
#include <cmath>

struct Quaternion
{
	float x, y, z, w;
};

inline Quaternion operator/(const Quaternion& q1, float& n) {
	Quaternion result;

	result.w = q1.w / n;
	result.x = q1.x / n;
	result.y = q1.y / n;
	result.z = q1.z / n;
	return result;
}

inline Quaternion operator-(const Quaternion& q1, float& n) {
	Quaternion result;

	result.w = q1.w - n;
	result.x = q1.x - n;
	result.y = q1.y - n;
	result.z = q1.z - n;
	return result;
}

inline Quaternion operator*(const Quaternion q1, float n) {
	Quaternion result;

	result.w = q1.w * n;
	result.x = q1.x * n;
	result.y = q1.y * n;
	result.z = q1.z * n;
	return result;
}

inline Quaternion operator*(float n, Quaternion q1) {
	Quaternion result;

	result.w = q1.w * n;
	result.x = q1.x * n;
	result.y = q1.y * n;
	result.z = q1.z * n;
	return result;
}

inline Quaternion operator+(const Quaternion& q1, float n) {
	Quaternion result;

	result.w = q1.w + n;
	result.x = q1.x + n;
	result.y = q1.y + n;
	result.z = q1.z + n;
	return result;
}

inline Quaternion operator-(const Quaternion& q1) {
	Quaternion result;

	result.w = -q1.w;
	result.x = -q1.x;
	result.y = -q1.y;
	result.z = -q1.z;
	return result;
}

inline Quaternion operator-=(Quaternion& q1, const Quaternion& q2) {
	q1.w -= q2.w;
	q1.x -= q2.x;
	q1.y -= q2.y;
	q1.z -= q2.z;
	return q1;
}

inline Quaternion operator+(Quaternion q1, Quaternion q2) {
	q1.w += q2.w;
	q1.x += q2.x;
	q1.y += q2.y;
	q1.z += q2.z;
	return q1;
}

inline Quaternion operator*(Quaternion q1, Quaternion q2) {
	q1.w *= q2.w;
	q1.x *= q2.x;
	q1.y *= q2.y;
	q1.z *= q2.z;
	return q1;
}
