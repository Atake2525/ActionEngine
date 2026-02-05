#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

#pragma once
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

	// デフォルト値
    static const Transform Default;
};

inline const Transform Transform::Default = {
	Vector3{1.0f, 1.0f, 1.0f}, // scale
	Vector3{0.0f, 0.0f, 0.0f}, // rotate
	Vector3{0.0f, 0.0f, 0.0f}  // translate
};

struct QuaternionTransform
{
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};

inline Transform operator*=(Transform& t, const float& n) {
	t.scale *= n;
	t.rotate *= n;
	t.translate *= n;
	return t;
}

inline Transform operator*=(Transform& t1, const Transform& t2) {
	t1.scale *= t2.scale;
	t1.rotate *= t2.rotate;
	t1.translate *= t2.translate;
	return t1;
}