#include "Camera.h"
#include "kMath.h"
#include "WinApp.h"
#include "ImGuiManager.h"

Camera::Camera(WinApp& winApp)
	: m_pWinApp(&winApp)
	, transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspect(float(m_pWinApp->GetkClientWidth()) / float(m_pWinApp->GetkClientHeight()))
	, nearClipDistance(0.1f)
	, farClipDistance(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.position))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePrespectiveFovMatrix(fovY, aspect, nearClipDistance, farClipDistance))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
    , drawHeihgt(1.0f)
{
}

void Camera::Update() {

	transform.rotate.x = std::fmod(transform.rotate.x, 2 * std::numbers::pi_v<float>);
	transform.rotate.y = std::fmod(transform.rotate.y, 2 * std::numbers::pi_v<float>);
	transform.rotate.z = std::fmod(transform.rotate.z, 2 * std::numbers::pi_v<float>);

	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.position);
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
	aspect = float(m_pWinApp->GetkClientWidth()) / float(m_pWinApp->GetkClientHeight());
	projectionMatrix = MakePrespectiveFovMatrix(fovY, aspect, nearClipDistance, farClipDistance);

	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

	Vector3 cameraRightDir = {
		std::cos(transform.rotate.y),
		0.0f,
		std::sin(transform.rotate.y)
	};

#ifndef NDEBUG
	Vector3 rotate = SwapDegree(transform.rotate);

	Vector3 dirc = { -worldMatrix.m[0][2], -worldMatrix.m[1][2], worldMatrix.m[2][2] };

	ImGui::SetNextWindowPos(ImVec2{ float(m_pWinApp->GetkClientWidth()) - 300.0f, 18.0f * number }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2{ 300.0f, 128.0f }, ImGuiCond_FirstUseEver);
	ImGui::Begin("CameraStatus");
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
	ImGui::DragFloat3("Rotation", &rotate.x, 0.5f);
	ImGui::DragFloat3("Direction1", &direction.x);
	ImGui::DragFloat3("Direction2", &dirc.x);
	ImGui::DragFloat3("Direction3", &cameraRightDir.x);
	ImGui::DragFloat("Fov", &fovY, 0.01f);
	ImGui::DragFloat("farClipDist", &farClipDistance, 1.0f);
	ImGui::DragFloat("drawHeihgt", &drawHeihgt, 0.1f);
	ImGui::End();

	transform.rotate = SwapRadian(rotate);
#endif // _CAMERADEBUG
}

const Vector3 Camera::GetWorldPosition() const
{
	Vector3 result;
	result.x = worldMatrix.m[3][0];
	result.y = worldMatrix.m[3][1];
	result.z = worldMatrix.m[3][2];
	return result;
}

const Vector3 Camera::GetHolizontalDirection() const {
	return { std::cos(transform.rotate.y), 0.0f, std::sin(transform.rotate.y) };
}