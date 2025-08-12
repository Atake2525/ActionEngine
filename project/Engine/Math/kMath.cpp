#include "kMath.h"
#include <algorithm>

const Vector3 operator*(const Vector3& v, const float f) {
	Vector3 result;
	result.x = v.x * f;
	result.y = v.y * f;
	result.z = v.z * f;
	return result;
}

//Vector3& operator+=(Vector3& v1, const Vector3& v2) {
//	v1.x += v2.x;
//	v1.y += v2.y;
//	v1.z += v2.z;
//	return v1;
//}

//const Vector3 operator+(const Vector3& v1, const Vector3 v2) {
//	Vector3 result;
//	result.x = v1.x + v2.x;
//	result.y = v1.y + v2.y;
//	result.z = v1.z + v2.z;
//	return result;
//}

//Vector3& operator-=(Vector3& v1, const Vector3& v2) {
//	v1.x -= v2.x;
//	v1.y -= v2.y;
//	v1.z -= v2.z;
//	return v1;
//}

//const Vector3 operator-(Vector3& v1, const Vector3 v2) {
//	Vector3 result;
//	result.x = v1.x - v2.x;
//	result.y = v1.y - v2.y;
//	result.z = v1.z - v2.z;
//	return result;
//}

//Vector3& operator*=(Vector3& v1, const Vector3& v2) {
//	v1.x *= v2.x;
//	v1.y *= v2.y;
//	v1.z *= v2.z;
//	return v1;
//}

const Vector3 operator*(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x * v2.x;
	result.y = v1.y * v2.y;
	result.z = v1.z * v2.z;
	return result;
}

Vector3& operator/=(Vector3& v1, const Vector3& v2) {
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	return v1;
}

const Vector3 operator/(const Vector3& v1, const Vector3 v2) {
	Vector3 result;
	result.x = v1.x / v2.x;
	result.y = v1.y / v2.y;
	result.z = v1.z / v2.z;
	return result;
}

const Vector3 operator+(const Vector3& v1, const float f) {
	Vector3 result;
	result.x = v1.x / f;
	result.y = v1.y / f;
	result.z = v1.z / f;
	return result;
}

const Vector3 operator-(const Vector3& v1, const float f) {
	Vector3 result;
	result.x = v1.x / f;
	result.y = v1.y / f;
	result.z = v1.z / f;
	return result;
}

const Vector3 operator/(const Vector3& v1, const float f) {
	Vector3 result;
	result.x = v1.x / f;
	result.y = v1.y / f;
	result.z = v1.z / f;
	return result;
}

//const Vector3 operator-(const Vector3& v1) {
//	Vector3 v;
//	v -= v1;
//	return v;
//}

Vector3 Vector3::operator-(const Vector3& other) const {
	return { x - other.x, y - other.y, z - other.z };
}

//単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 ans = { 0 };
	for (int a = 0; a < 4; a++) {
		for (int b = 0; b < 4; b++) {
			if (a == b)
			{
				ans.m[a][b] = 1;
			}
		}
	}
	return ans;
};

// 1, 2次元の軸回転行列
Matrix3x3 MakeRotateMatrix3x3(float radian) {
	Matrix3x3 ans = { 0 };
	ans.m[0][0] = std::cos(radian);
	ans.m[0][1] = std::sin(radian);
	ans.m[1][0] = -std::sin(radian);
	ans.m[1][1] = -std::cos(radian);
	ans.m[2][2] = 1;
	return ans;
};


//  行列の積
Matrix3x3 Multiply3x3(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 ans;
	for (int a = 0; a < 2; a++) {
		for (int b = 0; b < 2; b++) {
			ans.m[a][b] = m1.m[a][0] * m2.m[0][b] + m1.m[a][1] * m2.m[1][b] + m1.m[a][2] * m2.m[2][b];
		}
	}
	return ans;
};

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 ans;
	ans.x = v1.y * v2.z - v1.z * v2.y;
	ans.y = v1.z * v2.x - v1.x * v2.z;
	ans.z = v1.x * v2.y - v1.y * v2.x;
	return ans;
};

float Dot(const Vector3& v1, const Vector3& v2) {
	float ans;
	ans = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return ans;
};

// 共役Quaternionを返す
Quaternion Conjugate(const Quaternion& quaternion) {
	Quaternion result;
	result = quaternion;
	result.x = quaternion.x * -1;
	result.y = quaternion.y * -1;
	result.z = quaternion.z * -1;
	return result;
}

// Quaternionの積
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	Quaternion result;
	result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
	result.x = (lhs.y * rhs.z - lhs.z * rhs.y + rhs.w * lhs.x + lhs.w * rhs.x);
	result.y = (lhs.z * rhs.x - lhs.x * rhs.z + rhs.w * lhs.y + lhs.w * rhs.y);
	result.z = (lhs.x * rhs.y - lhs.y * rhs.x + rhs.w * lhs.z + lhs.w * rhs.z);
	return result;
}

// 任意軸回転を表すQuaternionの生成
Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Quaternion result;
	result.w = cos(angle / 2);
	result.x = axis.x * sin(angle / 2);
	result.y = axis.y * sin(angle / 2);
	result.z = axis.z * sin(angle / 2);
	return result;
}

// ベクトルをQuaternionで回転させた結果のベクトルを求める
Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	Vector3 result;
	Quaternion r{ .x = vector.x, .y = vector.y, .z = vector.z, .w = 0 };
	Quaternion r1 = Multiply(Multiply(quaternion, r), Conjugate(quaternion));
	result.x = r1.x;
	result.y = r1.y;
	result.z = r1.z;
	return result;
}

// Quaternionから回転行列を求める
Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
	Matrix4x4 result;
	result.m[0][0] = (quaternion.w * quaternion.w) + (quaternion.x * quaternion.x) - (quaternion.y * quaternion.y) - (quaternion.z * quaternion.z);
	result.m[0][1] = 2 * (quaternion.x * quaternion.y + quaternion.w * quaternion.z);
	result.m[0][2] = 2 * (quaternion.x * quaternion.z - quaternion.w * quaternion.y);
	result.m[0][3] = 0.0f;
	result.m[1][0] = 2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z);
	result.m[1][1] = (quaternion.w * quaternion.w) - (quaternion.x * quaternion.x) + (quaternion.y * quaternion.y) - (quaternion.z * quaternion.z);
	result.m[1][2] = 2 * (quaternion.y * quaternion.z + quaternion.w * quaternion.x);
	result.m[1][3] = 0.0f;
	result.m[2][0] = 2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y);
	result.m[2][1] = 2 * (quaternion.y * quaternion.z - quaternion.w * quaternion.x);
	result.m[2][2] = (quaternion.w * quaternion.w) - (quaternion.x * quaternion.x) - (quaternion.y * quaternion.y) + (quaternion.z * quaternion.z);
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle) {

	// 資料p20を参考に中身を埋める。nはaxisのこと
	float cos = std::cos(angle);
	float sin = std::sin(angle);

	Matrix4x4 rotateMatrix = {};
	rotateMatrix.m[0][0] = axis.x * axis.x * (1 - cos) + cos;
	rotateMatrix.m[0][1] = axis.x * axis.y * (1 - cos) + axis.z * sin;
	rotateMatrix.m[0][2] = axis.x * axis.z * (1 - cos) - axis.y * sin;
	rotateMatrix.m[0][3] = 0;

	rotateMatrix.m[1][0] = axis.x * axis.y * (1 - cos) - axis.z * sin;
	rotateMatrix.m[1][1] = axis.y * axis.y * (1 - cos) + cos;
	rotateMatrix.m[1][2] = axis.y * axis.z * (1 - cos) + axis.x * sin;
	rotateMatrix.m[1][3] = 0.0f;

	rotateMatrix.m[2][0] = axis.x * axis.z * (1 - cos) + axis.y * sin;
	rotateMatrix.m[2][1] = axis.y * axis.z * (1 - cos) - axis.x * sin;
	rotateMatrix.m[2][2] = axis.z * axis.z * (1 - cos) + cos;
	rotateMatrix.m[2][3] = 0.0f;

	rotateMatrix.m[3][0] = 0.0f;
	rotateMatrix.m[3][1] = 0.0f;
	rotateMatrix.m[3][2] = 0.0f;
	rotateMatrix.m[3][3] = 1.0f;

	return rotateMatrix;
}

//2次元アフィン変換行列
Matrix3x3 MakeAffineMatrix3x3(const Vector2& scale, const Vector2& rotate, const Vector2& translate) {
	Matrix3x3 S = { 0 };
	Matrix3x3 R = { 0 };
	Matrix3x3 T = { 0 };
	Matrix3x3 ans = { 0 };

	S.m[0][0] = scale.x;
	S.m[1][1] = scale.y;
	S.m[2][2] = 1;

	R.m[0][0] = std::cos(rotate.x);
	R.m[1][0] = -std::sin(rotate.y);
	R.m[0][1] = std::sin(rotate.x);
	R.m[1][1] = std::cos(rotate.y);

	T.m[0][0] = 1;
	T.m[1][1] = 1;
	T.m[2][2] = 1;
	T.m[2][0] = translate.x;
	T.m[2][1] = translate.y;

	ans.m[0][0] = S.m[0][0] * R.m[0][0];
	ans.m[0][1] = S.m[0][1] * R.m[0][1];
	ans.m[1][0] = S.m[1][0] * R.m[1][0];
	ans.m[1][1] = S.m[1][1] * R.m[1][1];
	ans.m[2][0] = T.m[2][0];
	ans.m[2][1] = T.m[2][1];
	ans.m[2][2] = 1;

	return ans;
};

// 1, x軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = 1;
	ans.m[1][1] = std::cos(radian);
	ans.m[1][2] = std::sin(radian);
	ans.m[2][1] = -std::sin(radian);
	ans.m[2][2] = std::cos(radian);
	ans.m[3][3] = 1;

	return ans;
};

// 2, y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = std::cos(radian);
	ans.m[0][2] = -std::sin(radian);
	ans.m[1][1] = 1;
	ans.m[2][0] = std::sin(radian);
	ans.m[2][2] = std::cos(radian);
	ans.m[3][3] = 1;

	return ans;
};

// 3, z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = std::cos(radian);
	ans.m[0][1] = std::sin(radian);
	ans.m[1][0] = -std::sin(radian);
	ans.m[1][1] = std::cos(radian);
	ans.m[2][2] = 1;
	ans.m[3][3] = 1;

	return ans;
};

//座標変換
Vector3 MatrixTransform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 ans;

	ans.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	ans.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	ans.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	ans.x /= w;
	ans.y /= w;
	ans.z /= w;

	return ans;
}

// Scale計算
Matrix4x4 MakeScaleMatrix(Vector3 scale) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = scale.x;
	ans.m[1][1] = scale.y;
	ans.m[2][2] = scale.z;
	ans.m[3][3] = 1;

	return ans;
};

// Translate計算
Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = 1;
	ans.m[1][1] = 1;
	ans.m[2][2] = 1;
	ans.m[3][3] = 1;
	ans.m[3][0] = translate.x;
	ans.m[3][1] = translate.y;
	ans.m[3][2] = translate.z;

	return ans;
};

//  行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 ans;
	for (int a = 0; a < 4; a++) {
		for (int b = 0; b < 4; b++) {
			ans.m[a][b] = m1.m[a][0] * m2.m[0][b] + m1.m[a][1] * m2.m[1][b] + m1.m[a][2] * m2.m[2][b] + m1.m[a][3] * m2.m[3][b];
		}
	}
	return ans;
};

// 3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 R = { 0 };
	Matrix4x4 ans = { 0 };

	R = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));


	ans.m[0][0] = scale.x * R.m[0][0];
	ans.m[0][1] = scale.x * R.m[0][1];
	ans.m[0][2] = scale.x * R.m[0][2];
	ans.m[1][0] = scale.y * R.m[1][0];
	ans.m[1][1] = scale.y * R.m[1][1];
	ans.m[1][2] = scale.y * R.m[1][2];
	ans.m[2][0] = scale.z * R.m[2][0];
	ans.m[2][1] = scale.z * R.m[2][1];
	ans.m[2][2] = scale.z * R.m[2][2];
	ans.m[3][3] = 1;
	ans.m[3][0] = translate.x;
	ans.m[3][1] = translate.y;
	ans.m[3][2] = translate.z;

	return ans;
}
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate)
{
	Matrix4x4 R = { 0 };
	Matrix4x4 ans = { 0 };

	//R = Multiply(MakeRotateXMatrix(rotate.x), Multiply(MakeRotateYMatrix(rotate.y), MakeRotateZMatrix(rotate.z)));

	R = MakeRotateMatrix(rotate);

	ans.m[0][0] = scale.x * R.m[0][0];
	ans.m[0][1] = scale.x * R.m[0][1];
	ans.m[0][2] = scale.x * R.m[0][2];
	ans.m[1][0] = scale.y * R.m[1][0];
	ans.m[1][1] = scale.y * R.m[1][1];
	ans.m[1][2] = scale.y * R.m[1][2];
	ans.m[2][0] = scale.z * R.m[2][0];
	ans.m[2][1] = scale.z * R.m[2][1];
	ans.m[2][2] = scale.z * R.m[2][2];
	ans.m[3][3] = 1;
	ans.m[3][0] = translate.x;
	ans.m[3][1] = translate.y;
	ans.m[3][2] = translate.z;

	return ans;
}
Matrix4x4 MakeAffineMatrix(Transform transform)
{
	return MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
}
Matrix4x4 MakeAffineMatrix(QuaternionTransform quaternionTransform)
{
	return MakeAffineMatrix(quaternionTransform.scale, quaternionTransform.rotate, quaternionTransform.translate);
}
;

//３次元アフィン変換行列
Matrix4x4 MakeAffineMatrixInQuaternion(const Vector3& scale, const Matrix4x4& axisAngle, const Vector3& translate) {
	Matrix4x4 R = axisAngle;
	Matrix4x4 ans = { 0 };


	ans.m[0][0] = scale.x * R.m[0][0];
	ans.m[0][1] = scale.x * R.m[0][1];
	ans.m[0][2] = scale.x * R.m[0][2];
	ans.m[1][0] = scale.y * R.m[1][0];
	ans.m[1][1] = scale.y * R.m[1][1];
	ans.m[1][2] = scale.y * R.m[1][2];
	ans.m[2][0] = scale.z * R.m[2][0];
	ans.m[2][1] = scale.z * R.m[2][1];
	ans.m[2][2] = scale.z * R.m[2][2];
	ans.m[3][3] = 1;
	ans.m[3][0] = translate.x;
	ans.m[3][1] = translate.y;
	ans.m[3][2] = translate.z;

	return ans;
};

//逆行列
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 ans;
	float inverse;
	inverse = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	ans.m[0][0] = 1 / inverse * (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
	ans.m[0][1] = 1 / inverse * (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);
	ans.m[0][2] = 1 / inverse * (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
	ans.m[0][3] = 1 / inverse * (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);

	ans.m[1][0] = 1 / inverse * (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);
	ans.m[1][1] = 1 / inverse * (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
	ans.m[1][2] = 1 / inverse * (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);
	ans.m[1][3] = 1 / inverse * (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

	ans.m[2][0] = 1 / inverse * (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
	ans.m[2][1] = 1 / inverse * (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);
	ans.m[2][2] = 1 / inverse * (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
	ans.m[2][3] = 1 / inverse * (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);

	ans.m[3][0] = 1 / inverse * (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);
	ans.m[3][1] = 1 / inverse * (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
	ans.m[3][2] = 1 / inverse * (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);
	ans.m[3][3] = 1 / inverse * (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

	return ans;
};

// 正規化
Vector3 Normalize(const Vector3& v) {
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (length == 0) {
		// 長さがゼロの場合、ゼロベクトルを返す
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	else {
		return Vector3(v.x / length, v.y / length, v.z / length);
	}
};


// 1, 透視投影行列
Matrix4x4 MakePrespectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 ans = { 0 };

	float cot = 1 / std::tan(fovY / 2);
	ans.m[0][0] = (1 / aspectRatio) * cot;
	ans.m[1][1] = cot;
	ans.m[2][2] = farClip / (farClip - nearClip);
	ans.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	ans.m[2][3] = 1;


	return ans;
};

// 2, 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = 2 / (right - left);
	ans.m[1][1] = 2 / (top - bottom);
	ans.m[2][2] = 1 / (farClip - nearClip);
	ans.m[3][0] = (left + right) / (left - right);
	ans.m[3][1] = (top + bottom) / (bottom - top);
	ans.m[3][2] = nearClip / (nearClip - farClip);
	ans.m[3][3] = 1;

	return ans;
};

// 3, ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 ans = { 0 };

	ans.m[0][0] = width / 2;
	ans.m[1][1] = height / 2 * -1;
	ans.m[2][2] = maxDepth - minDepth;
	ans.m[3][0] = left + width / 2;
	ans.m[3][1] = top + height / 2;
	ans.m[3][2] = minDepth;
	ans.m[3][3] = 1;

	return ans;
};

Vector3 SwapDegree(Vector3 radian) {
	Vector3 result = {
		radian.x * (180.0f / float(M_PI)),
		radian.y * (180.0f / float(M_PI)),
		radian.z * (180.0f / float(M_PI)),
	};
	return result;
}

Vector3 SwapRadian(Vector3 degree) {
	Vector3 result = {
		degree.x * (float(M_PI) / 180.0f),
		degree.y * (float(M_PI) / 180.0f),
		degree.z * (float(M_PI) / 180.0f),
	};
	return result;
}

Vector2 SwapDegree(Vector2 radian) {
	Vector2 result = {
		radian.x * (180.0f / float(M_PI)),
		radian.y * (180.0f / float(M_PI)),
	};
	return result;
}

Vector2 SwapRadian(Vector2 degree) {
	Vector2 result = {
		degree.x * (float(M_PI) / 180.0f),
		degree.y * (float(M_PI) / 180.0f),
	};
	return result;
}

float SwapDegree(float radian) {
	float result = radian * (180.0f / float(M_PI));
	return result;
}

float SwapRadian(float degree) {
	float result = degree * (float(M_PI) / 180.0f);
	return result;
}

float Length(const Vector3& v) {
	float result = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return result;
}

float Distance(const Vector3& v1, const Vector3& v2) {
	Vector3 dist = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	float result = Length(dist);
	return result;
}

const Vector3 CenterAABB(const AABB& aabb)
{
	Vector3 result = (aabb.max - aabb.min) / 2 + aabb.min;
	return result;
}

Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
	Vector3 result{
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2],
	};
	return result;
}

// ease In-Out x1 : 開始点  x2 : 目標点
float easeInOut(float time, float x1, float x2) {
	float T = time;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = -(cos(float(M_PI * T)) - 1.0f) / 2.0f;

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// ease In-Out x1 : 開始点  x2 : 目標点
int easeInOut(float t, unsigned int x1, unsigned int x2) {
	float T = t;
	T = std::clamp(T, 0.0f, 1.0f);
	int x;
	// easeOut
	int easedT = -int((cos(float(M_PI * int(T))) - 1.0f) / 2.0f);

	x = (1 - easedT) * x1 + easedT * x2;
	return x;
};

// easeOutQuint
float easeOutQuint(float t, float x1, float x2) {
	float T = t;
	T = std::clamp(T, 0.0f, 1.0f);
	float x;
	// easeOut
	float easedT = 1.0f - pow(1.0f - T, 5.0f);

	x = (1.0f - easedT) * x1 + easedT * x2;
	return x;
};

// easeInBack
float easeInBack(float t, float x1, float x2) {
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
Vector3 easeInOut(float time, Vector3 x1, Vector3 x2) {

	Vector3 result;

	result.x = easeInOut(time, x1.x, x2.x);
	result.y = easeInOut(time, x1.y, x2.y);
	result.z = easeInOut(time, x1.z, x2.z);

	return result;;
};

// easeOutQuint
Vector3 easeOutQuint(float t, Vector3 x1, Vector3 x2) {

	Vector3 result;

	result.x = easeOutQuint(t, x1.x, x2.x);
	result.y = easeOutQuint(t, x1.y, x2.y);
	result.z = easeOutQuint(t, x1.z, x2.z);

	return result;;
};

// easeInBack
Vector3 easeInBack(float t, Vector3 x1, Vector3 x2) {
	Vector3 result;

	result.x = easeInBack(t, x1.x, x2.x);
	result.y = easeInBack(t, x1.y, x2.y);
	result.z = easeInBack(t, x1.z, x2.z);

	return result;;
};

Quaternion Slerp(const Quaternion& befor, const Quaternion& after, float t) {
	//float dot = Dot(q0, q1);
	//
	//Quaternion quat1 = q1;

	//// 最短経路補正（反転処理）
	////Quaternion q1Mod = (dot < 0.0f) ? -q1 : q1;
	//if (dot < 0.0f)
	//{
	//	quat1 = -quat1;
	//}
	//dot = std::clamp(dot, -1.0f, 1.0f); // acosの定義域に収める

	//const float epsilon = 1e-6f;
	//if (dot > 1.0f - epsilon) {
	//	// ほぼ同じ方向なら線形補間で近似
	//	return QuaternionNormalize(q0 * (1.0f - t) + quat1 * t);
	//}

	//float theta = std::acos(dot);
	//float sinTheta = std::sin(theta);
	//float scale0 = std::sin((1.0f - t) * theta) / sinTheta;
	//float scale1 = std::sin(t * theta) / sinTheta;

	//Quaternion result = q0 * scale0 + quat1 * scale1;
	//return QuaternionNormalize(result);

	Quaternion quat0 = befor;
	Quaternion quat1 = after;


	float dot = Dot(quat0, quat1);
	if (dot < 0.0f)
	{
		quat1 = -quat1;
		//dot = -dot;
	}

	dot = std::clamp(dot, -1.0f, 1.0f);

	const float epsilon = 1e-6f;

	if (dot > 1.0f - epsilon)
	{
		return QuaternionNormalize((1.0f - t) * quat0 + t * quat1);
	}

	// なす角を求める
	float theta = std::acos(dot);

	float scale0 = sin((1.0f - t) * theta) / sin(theta);
	float scale1 = sin(t * theta) / sin(theta);

	return QuaternionNormalize(scale0 * quat0 + scale1 * quat1);


	//float time = t;

	//Quaternion q = q0;
	//Quaternion result;
	//float dot = Dot(q0, q1);

	//if (dot > 0.0f) { // 符号逆にしているからどこかで問題が起こる可能性あり
	//	q = Inverse(q0);
	//	dot = -dot;
	//}

	//dot = std::clamp(dot, -1.0f, 1.0f);


	//// なす角を求める
	//float theta = std::acos(dot);

	//float scale0 = sin((1 - time) * theta) / sin(theta);
	//float scale1 = sin(time * theta) / sin(theta);

	//result.x = scale0 * q.x + scale1 * q1.x;
	//result.y = scale0 * q.y + scale1 * q1.y;
	//result.z = scale0 * q.z + scale1 * q1.z;
	//result.w = scale0 * q.w + scale1 * q1.w;

	//assert(!std::isnan(result.x));
	//assert(!std::isnan(result.y));
	//assert(!std::isnan(result.z));
	//assert(!std::isnan(result.w));

	//if (result.w > 1.0f || result.w < -1.0f)
	//{
	//	assert(false);
	//}

	//return result;

}

Quaternion QuaternionNormalize(const Quaternion q) {
	float result = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	if (result == 0.0f) return { 0, 0, 0, 1 }; // 回転なしの単位クォータニオン

	return {
		q.x / result,
		q.y / result,
		q.z / result,
		q.w / result
	};

}

Vector3 Lerp(const Vector3& befor, const Vector3& after, float t) {
	Vector3 ans = { 0 };
	ans.x = t * after.x + (1.0f - t) * befor.x;
	ans.y = t * after.y + (1.0f - t) * befor.y;
	ans.z = t * after.z + (1.0f - t) * befor.z;
	return ans;
}

float Lerp(const float& befor, const float& after, float t) {
	float ans = 0.0f;
	ans = t * after + (1.0f - t) * befor;
	return ans;
}

float Dot(const Quaternion& v1, const Quaternion& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }

// 逆Quaternionを返す
Quaternion Inverse(const Quaternion& quaternion) {
	Quaternion result;
	Quaternion q = Conjugate(quaternion);
	float length = Norm(quaternion) * Norm(quaternion);
	assert(length != 0.0f);
	result = q / length;

	return result;
}

// Quaternionのnormを返す
float Norm(const Quaternion& quaternion) {
	float result;
	result = sqrtf((quaternion.w * quaternion.w) + (quaternion.x * quaternion.x) + (quaternion.y * quaternion.y) + (quaternion.z * quaternion.z));
	return result;
}

Vector3 ClosestPoint(const AABB& aabb, const Vector3& segment){

	Vector3 result;
	result.x = std::clamp(segment.x, aabb.min.x, aabb.max.x);
	result.y = std::clamp(segment.y, aabb.min.y, aabb.max.y);
	result.z = std::clamp(segment.z, aabb.min.z, aabb.max.z);

	return result;
}

float LengthSquared(const Vector3& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}
