#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#define NOMINMAX
#include <algorithm>

#pragma once

// ease In-Out x1 : 開始点  x2 : 目標点
inline float EaseInOut(float time, float x1, float x2) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = -(cos(float(M_PI * T)) - 1.0f) / 2.0f;

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// ease In-Out x1 : 開始点  x2 : 目標点
inline int EaseInOut(float t, int x1, int x2) {
	float T = t;
	T = std::clamp(T, 0.0f, 1.0f);
	int x;
	// easeOut
	int easedT = -int((cos(float(M_PI * int(T))) - 1.0f) / 2.0f);

	x = (1 - easedT) * x1 + easedT * x2;
	return x;
};

// easeOutQuint
inline float EaseOutQuint(float t, float x1, float x2) {
	float T = t;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = 1.0f - pow(1.0f - T, 5.0f);

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// easeInBack
inline float EaseInBack(float t, float x1, float x2) {
	float T = t;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	const float c1 = 1.70158f;
	const float c3 = c1 + 1;
	// easeOut
	float easedT = c3 * T * T * T - c1 * T * T;

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// ease In-Out x1 : 開始点  x2 : 目標点
inline Vector3 EaseInOut(float time, Vector3 x1, Vector3 x2) {

	Vector3 result;

	result.x = EaseInOut(time, x1.x, x2.x);
	result.y = EaseInOut(time, x1.y, x2.y);
	result.z = EaseInOut(time, x1.z, x2.z);

	return result;;
};

// easeOutQuint
inline Vector3 EaseOutQuint(float t, Vector3 x1, Vector3 x2) {

	Vector3 result;

	result.x = EaseOutQuint(t, x1.x, x2.x);
	result.y = EaseOutQuint(t, x1.y, x2.y);
	result.z = EaseOutQuint(t, x1.z, x2.z);

	return result;;
};

// easeInBack
inline Vector3 EaseInBack(float t, Vector3 x1, Vector3 x2) {
	Vector3 result;

	result.x = EaseInBack(t, x1.x, x2.x);
	result.y = EaseInBack(t, x1.y, x2.y);
	result.z = EaseInBack(t, x1.z, x2.z);

	return result;;
};

inline float EaseOutExpo(float time, float start, float end) {
	float result;
	float timer = std::clamp(time, 0.0f, 1.0f);
	if (timer == 1.0f) {
		result = end;
	}
	else {
		float easedT = 1 - pow(2, -10 * timer);
		result = (1.0f - easedT) * start + easedT * end;
	}
	return result;
}

inline Vector2 EaseInExpo(float time, Vector2 start, Vector2 end) {
	Vector2 result;

	result.x = EaseInBack(time, start.x, end.x);
	result.y = EaseInBack(time, start.y, end.y);

	return result;
}

inline Vector3 EaseOutExpo(float time, Vector3 start, Vector3 end) {
	Vector3 result;

	result.x = EaseOutExpo(time, start.x, end.x);
	result.y = EaseOutExpo(time, start.y, end.y);
	result.z = EaseOutExpo(time, start.z, end.z);

	return result;
}