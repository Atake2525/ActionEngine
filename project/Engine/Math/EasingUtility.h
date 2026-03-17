#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <algorithm>

#pragma once

// ease In-Out x1 : 開始点  x2 : 目標点
inline float EaseInOut(float x1, float x2, float time) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = -(cos(float(M_PI * T)) - 1.0f) / 2.0f;

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// ease In-Out x1 : 開始点  x2 : 目標点
inline int EaseInOut(int x1, int x2, float time) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	int x;
	// easeOut
	int easedT = -int((cos(float(M_PI * int(T))) - 1.0f) / 2.0f);

	x = (1 - easedT) * x1 + easedT * x2;
	return x;
};

// easeOutQuint
inline float EaseOutQuint(float x1, float x2, float time) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = 1.0f - pow(1.0f - T, 5.0f);

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// easeInBack
inline float EaseInBack(float x1, float x2, float time) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	// easeOut
	float easedT = c3 * T * T * T - c1 * T * T;

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// ease In-Out x1 : 開始点  x2 : 目標点
inline Vector3 EaseInOut(Vector3 x1, Vector3 x2, float time) {

	Vector3 result;

	result.x = EaseInOut(x1.x, x2.x, time);
	result.y = EaseInOut(x1.y, x2.y, time);
	result.z = EaseInOut(x1.z, x2.z, time);

	return result;;
};

// easeOutQuint
inline Vector2 EaseOutQuint(Vector2 x1, Vector2 x2, float time) {

	Vector2 result;

	result.x = EaseOutQuint(x1.x, x2.x, time);
	result.y = EaseOutQuint(x1.y, x2.y, time);

	return result;;
};

// easeOutQuint
inline Vector3 EaseOutQuint(Vector3 x1, Vector3 x2, float time) {

	Vector3 result;

	result.x = EaseOutQuint(x1.x, x2.x, time);
	result.y = EaseOutQuint(x1.y, x2.y, time);
	result.z = EaseOutQuint(x1.z, x2.z, time);

	return result;;
};

// easeInBack
inline Vector3 EaseInBack(Vector3 x1, Vector3 x2, float time) {
	Vector3 result;

	result.x = EaseInBack(x1.x, x2.x, time);
	result.y = EaseInBack(x1.y, x2.y, time);
	result.z = EaseInBack(x1.z, x2.z, time);

	return result;;
};

inline float EaseOutExpo(float start, float end, float time) {
	float result;
	float timer = std::clamp(time, 0.0f, 1.0f);
	if (timer == 1.0f) {
		result = end;
	}
	else {
		float easedT = 1.0f - pow(2.0f, -10.0f * timer);
		result = (1.0f - easedT) * start + easedT * end;
	}
	return result;
}

inline Vector2 EaseInExpo(Vector2 start, Vector2 end, float time) {
	Vector2 result;

	result.x = EaseInBack(start.x, end.x, time);
	result.y = EaseInBack(start.y, end.y, time);

	return result;
}

inline Vector3 EaseOutExpo(Vector3 start, Vector3 end, float time) {
	Vector3 result;

	result.x = EaseOutExpo(start.x, end.x, time);
	result.y = EaseOutExpo(start.y, end.y, time);
	result.z = EaseOutExpo(start.z, end.z, time);

	return result;
}

inline float EaseOutElastic(float start, float end, float time) {
	float t = std::clamp(time, 0.0f, 1.0f);
	float result;

	float c4 = (2.0f * float(M_PI)) / 3.0f;

	if (t == 1.0f)
	{
		result = end;
	}
	else
	{
		result = std::pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
	}

	return result;
}

inline Vector2 EaseOutElastic(Vector2 start, Vector2 end, float time) {
	Vector2 result;

	result.x = EaseOutElastic(start.x, end.x, time);
	result.y = EaseOutElastic(start.y, end.y, time);

	return result;
}

inline Vector3 EaseOutElastic(Vector3 start, Vector3 end, float time) {
	Vector3 result;

	result.x = EaseOutElastic(start.x, end.x, time);
	result.y = EaseOutElastic(start.y, end.y, time);
	result.z = EaseOutElastic(start.z, end.z, time);

	return result;
}

inline Vector3 CatmullRomTangent(const Vector3& P0, const Vector3& P1, const Vector3& P2, const Vector3& P3, float t)
{
	float t2 = t * t;

	return 0.5f * (
		(-P0 + P2) +
		(4.0f * P0 - 10.0f * P1 + 8.0f * P2 - 2.0f * P3) * t +
		(-3.0f * P0 + 9.0f * P1 - 9.0f * P2 + 3.0f * P3) * t2
		);
}