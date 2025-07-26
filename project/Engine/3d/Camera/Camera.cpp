#include "Camera.h"
#include "kMath.h"
#include "WinApp.h"
#include "ImGuiManager.h"

Camera::Camera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspect(float(WinApp::GetInstance()->GetkClientWidth()) / float(WinApp::GetInstance()->GetkClientHeight()))
	, nearClipDistance(0.1f)
	, farClipDistance(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePrespectiveFovMatrix(fovY, aspect, nearClipDistance, farClipDistance))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{
}

void Camera::Update() {

#ifdef _DEBUG
	Vector3 rotate = SwapDegree(transform.rotate);

	Vector3 dirc = { -worldMatrix.m[0][2], -worldMatrix.m[1][2], worldMatrix.m[2][2] };

	ImGui::Begin("CameraStatus");
	ImGui::SetWindowPos(ImVec2{ float(WinApp::GetInstance()->GetkClientWidth()) - 300.0f, 18.0f * number });
	ImGui::SetWindowSize(ImVec2{ 300.0f, 128.0f });
	ImGui::DragFloat3("Position", &transform.translate.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotate.x, 0.5f);
	ImGui::DragFloat3("Direction1", &direction.x);
	ImGui::DragFloat3("Direction2", &dirc.x);
	ImGui::DragFloat("Fov", &fovY, 0.01f);
	ImGui::DragFloat("farClipDist", &farClipDistance, 1.0f);
	ImGui::End();

	rotate = SwapRadian(rotate);
	transform.rotate = rotate;
#endif // _DEBUG


	transform.rotate.x = std::fmod(transform.rotate.x, 2 * std::numbers::pi_v<float>);
	transform.rotate.y = std::fmod(transform.rotate.y, 2 * std::numbers::pi_v<float>);
	transform.rotate.z = std::fmod(transform.rotate.z, 2 * std::numbers::pi_v<float>);

	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	if (isParent)
	{
		worldMatrix = Multiply(worldMatrix, parent);
	}
	else if (isTranslateParent)
	{
		Matrix4x4 translate = Multiply(worldMatrix, translateParent);
		worldMatrix.m[3][0] = translate.m[3][0];
		worldMatrix.m[3][1] = translate.m[3][1];
		worldMatrix.m[3][2] = translate.m[3][2];
	}
	else if (isRotateParent)
	{
		Matrix4x4 rotate = Multiply(worldMatrix, rotateParent);
		worldMatrix.m[0][0] = rotate.m[0][0];
		worldMatrix.m[0][1] = rotate.m[0][1];
		worldMatrix.m[0][2] = rotate.m[0][2];
		worldMatrix.m[1][0] = rotate.m[1][0];
		worldMatrix.m[1][1] = rotate.m[1][1];
		worldMatrix.m[1][2] = rotate.m[1][2];
		worldMatrix.m[2][0] = rotate.m[2][0];
		worldMatrix.m[2][1] = rotate.m[2][1];
		worldMatrix.m[2][2] = rotate.m[2][2];
	}

	direction = TransformNormal({ 0.0f, 0.0f, 1.0f }, worldMatrix);

	viewMatrix = Inverse(worldMatrix);
	projectionMatrix = MakePrespectiveFovMatrix(fovY, aspect, nearClipDistance, farClipDistance);
	// ここがエラーの可能性あり
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

const Vector3 Camera::GetWorldPosition() const
{
	Vector3 result;
	result.x = worldMatrix.m[3][0];
	result.y = worldMatrix.m[3][1];
	result.z = worldMatrix.m[3][2];
	return result;
}
