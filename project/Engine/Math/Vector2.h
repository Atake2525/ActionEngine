#include <cmath>
#pragma once

struct Vector2
{
	float x;
	float y;

};

inline Vector2 Normalize(const Vector2& v) {
	float length = std::sqrt(v.x * v.x + v.y * v.y);
	if (length == 0) {
		// 長さがゼロの場合、ゼロベクトルを返す
		return Vector2(0.0f, 0.0f);
	}
	else {
		return Vector2(v.x / length, v.y / length);
	}
}


inline Vector2 operator-(Vector2 v) {
	v.x -= v.x;
	v.y -= v.y;
	return v;
}

inline Vector2 operator-(Vector2 v1, Vector2 v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}


