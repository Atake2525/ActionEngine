#pragma once

#define ANCHORPOINT_LEFTTOP Vector2{0.0f, 0.0f}	// LEFTTOP
#define ANCHORPOINT_LEFTMIDDLE Vector2{0.0f, 0.5f}	// LEFTMIDDLE
#define ANCHORPOINT_LEFTBOTTOM Vector2{0.0f, 1.0f}	// LEFTBOTTOM
#define ANCHORPOINT_MIDDLETOP Vector2{0.5f, 0.0f}	// MIDDLETOP
#define ANCHORPOINT_MIDDLE Vector2{0.5f, 0.5f} // MIDDLE
#define ANCHORPOINT_MIDDLEBOTTOM Vector2{0.5f, 1.0f}	// MIDDLEBOTTOM
#define ANCHORPOINT_RIGHTTOP Vector2{1.0f, 0.0f}  // RIGHTTOP
#define ANCHORPOINT_RIGHTMIDDLE Vector2{1.0f, 0.5f}	// RIGHTMIDDLE
#define ANCHORPOINT_RIGHTBOTTOM Vector2{1.0f, 1.0f}	// RIGHTBOTTOM

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float pad[3];
	Matrix4x4 uvTransform;
	float shininess;
	Vector3 specularColor;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};