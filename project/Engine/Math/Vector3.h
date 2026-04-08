#pragma once

#include <algorithm>

struct Vector3
{
	float x;
	float y;
	float z;

	static const Vector3 Zero;
};

// 追加するオペレータの概要（目的: CatmullRom式などで使われる演算を有効にする）
// 1) 単項マイナス: -v をサポートする
// 2) スカラーとベクトルの乗算: float * Vector3 をサポートする（既存の Vector3 * float は存在）
// これにより式 (-P0 + P2) や (4.0f * P0) のような書き方がコンパイル通るようになる。

inline const Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }
inline const Vector3 operator*(float f, const Vector3& v) { return { v.x * f, v.y * f, v.z * f }; }

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

inline Vector3 operator+=(Vector3& v, const float& n) {
	v.x += n;
	v.y += n;
	v.z += n;
	return v;
}

inline Vector3 operator*=(Vector3& v, const Vector3& n) {
	v.x *= n.x;
	v.y *= n.y;
	v.z *= n.z;
	return v;
}

inline Vector3 operator-=(Vector3& v, const Vector3& n) {
	v.x -= n.x;
	v.y -= n.y;
	v.z -= n.z;
	return v;
}

inline Vector3 operator+=(Vector3& v, const Vector3& n) {
	v.x += n.x;
	v.y += n.y;
	v.z += n.z;
	return v;
}

inline const Vector3 operator-(const Vector3& a, const Vector3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline const Vector3 operator+(const Vector3& a, const Vector3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline const Vector3 operator*(const Vector3& a, const Vector3& b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
inline const Vector3 operator+(const Vector3& v, float f) { return { v.x + f, v.y + f, v.z + f }; }
inline const Vector3 operator-(const Vector3& v, float f) { return { v.x - f, v.y - f, v.z - f }; }
inline const Vector3 operator*(const Vector3& v, float f) { return { v.x * f, v.y * f, v.z * f }; }
inline const Vector3 operator-(float f, const Vector3& v) { return { f - v.x, f - v.y, f - v.z }; }


inline bool operator!=(const Vector3& v, const float f) {
	if (v.x != f && v.y != f && v.z != f)
	{
		return true;
	}
	return false;
}

inline bool operator!=(const Vector3& v1, const Vector3& v2) {
	if (v1.x != v2.x && v1.y != v2.y && v1.z != v2.z)
	{
		return true;
	}
	return false;
}

inline bool operator==(const Vector3& v, const float f) {
	if (v.x == f && v.y == f && v.z == f)
	{
		return true;
	}
	return false;
}

inline bool operator==(const Vector3& v1, const Vector3& v2) {
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
	{
		return true;
	}
	return false;
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

inline const Vector3 Vector3::Zero = Vector3{ 0.0f, 0.0f, 0.0f };