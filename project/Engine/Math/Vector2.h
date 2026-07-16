#include <cmath>
#pragma once

struct Vector2
{
	float x;
	float y;

	static const Vector2 Clamp(Vector2 target, Vector2 min, Vector2 max);
	static const Vector2 Clamp(Vector2 target, float min, float max);

    static const Vector2 Zero;
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

inline Vector2 operator+(Vector2 v1, Vector2 v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

inline Vector2 operator-(Vector2 v1, Vector2 v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

inline Vector2 operator*(const Vector2& v, float f) {
	return { v.x * f, v.y * f };
}

inline Vector2 operator*(const Vector2& v1, const Vector2& v2) {
	return { v1.x * v2.x, v1.y * v2.y };
}

inline Vector2 operator+=(Vector2& v, float f) {
	v.x += f;
	v.y += f;
	return v;
}

inline Vector2 operator*=(Vector2& v, float f) {
	v.x *= f;
	v.y *= f;
	return v;
}

inline Vector2 operator+=(const Vector2& v1, const Vector2& v2) {
	return { v1.x + v2.x, v1.y + v2.y };
}

inline Vector2 operator/=(const Vector2& v, float f) {
    return { v.x / f, v.y / f };
}

inline Vector2 operator/(const Vector2& v, float f) {
	return { v.x / f, v.y / f };
}

inline const Vector2 Vector2::Zero = Vector2{ 0.0f, 0.0f };