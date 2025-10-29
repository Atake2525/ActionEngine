#include "Transform.h"
#include "Matrix4x4.h"
#include "kMath.h"

struct CameraForGPU {
	Vector3 worldPosition;
};

#pragma once
class Camera {
public:
	Camera();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// Getter
	const Matrix4x4 GetWorldMatrix() const {
		return worldMatrix;
	}

	const Vector3 GetWorldPosition() const;
	// Getter
	const Matrix4x4& GetViewMatrix() const { return viewMatrix; }
	// Getter
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix; }
	// Getter
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix; }
	// Getter(Rotate)
	const Vector3& GetRotate() const { return transform.rotate; }
	// Getter(Translate)
	const Vector3& GetTranslate() const { return transform.translate; }
	// Getter(Direction)
	const Vector3& GetDirection() const { return direction; }
    // Getter(nearClipDistance)
    const float& GetNearClipDistance() const { return nearClipDistance; }
	// Getter(farClipDistance)
	const float& GetFarClipDistance() const { return farClipDistance; }
    // Getter(drawHeihgt)
    const float& GetDrawHeihgt() const { return drawHeihgt; }
	// Getter(fovY)
	const float& GetfovY() const { return fovY; }
	// Getter(Transform)
	const Transform& GetTransform() const { return transform; }
	// 任意軸回転の回転量を設定
	void SetAxisRotate(const Vector3& angle) { rotateAngle = angle; }

	const Vector3& GetAxisRotate() const { return rotateAngle; }

	// Setter(Rotate)
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	// Setter(Translate)
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	// Setter(farClipDistance)
	void SetFarClipDistance(const float& farClip) { farClipDistance = farClip; }
	// Setter(fovY)
	void SetFovY(const float& fov) { fovY = fov; }
	// Setter(Transform)
	void SetTransform(const Transform& transform) { this->transform = transform; }
	// Setter(Parent)
	void SetParent(const Matrix4x4& worldMatrix) {
		parent = worldMatrix;
		isParent = true;
		isTranslateParent = false;
		isRotateParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteParent() {
		isParent = false;
	}
	// Setter(Parent)
	void SetTranslateParent(const Matrix4x4& worldMatrix) {
		translateParent = worldMatrix;
		isTranslateParent = true;
		isParent = false;
		isRotateParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteTranslateParent() {
		isTranslateParent = false;
	}
	// Setter(Parent)
	void SetRotateParent(const Matrix4x4& worldMatrix) {
		rotateParent = worldMatrix;
		isRotateParent = true;
		isTranslateParent = false;
		isParent = false;
	}
	// Delete Parent(Parent 解除)
	void DeleteRotateParent() {
		isRotateParent = false;
	}

	int number = 0;

private:
	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Vector3 direction;

	Matrix4x4 parent;
	bool isParent = false;
	Matrix4x4 translateParent;
	bool isTranslateParent = false;
	Matrix4x4 rotateParent;
	bool isRotateParent = false;

	Matrix4x4 projectionMatrix;
	float fovY;
	float aspect;
	float nearClipDistance;
	float farClipDistance;

    float drawHeihgt;

	Vector3 axisAngle;
	Matrix4x4 rotateQuaternionMatrix;
	Vector3 rotateAngle;

	Matrix4x4 viewProjectionMatrix;
};
