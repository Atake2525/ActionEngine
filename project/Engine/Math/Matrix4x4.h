#pragma once

struct Matrix4x4
{
	float m[4][4];
};

inline Matrix4x4 Transpose(const Matrix4x4& matrix) {
	Matrix4x4 result;
	for (int k = 0; k < 4; k++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[j][k] = matrix.m[k][j];
		}
	}
	return result;
}
