#include "Light.h"
#include "kMath.h"
#include "DirectXBase.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

using namespace std;

void Light::Initialize(DirectXBase& dxBase) {
	m_pDirectXBase = &dxBase;

	// ライト関係の初期化
	directionalLightResource = m_pDirectXBase->CreateBufferResource(sizeof(DirectionalLight));

	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightData->specularColor = { 1.0f, 1.0f, 1.0f };

	pointLightResource = m_pDirectXBase->CreateBufferResource(sizeof(PointLight));

	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));

	pointLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->position = { 0.0f, 2.0f, 0.0f };
	pointLightData->intensity = 0.0f;
	pointLightData->radius = 25.0f;
	pointLightData->dacay = 5.0f;
	pointLightData->specularColor = { 1.0f, 1.0f, 1.0f };

	spotLightResource = m_pDirectXBase->CreateBufferResource(sizeof(SpotLight));

	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));

	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->position = { 10.0f, 1.25f, 0.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = Normalize({ 1.0f, 0.0f, 0.0f });
	spotLightData->intensity = 0.0f;
	spotLightData->dacay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 2.6f);
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->specularColor = { 1.0f, 1.0f, 1.0f };

	scanResource = m_pDirectXBase->CreateBufferResource(sizeof(ScanParam));
	scanResource->Map(0, nullptr, reinterpret_cast<void**>(&scanData));

	scanData->color = { 0.2f, 0.8f, 1.0f };
	scanData->width = 8.0f;
	scanData->radius = 0.1f;

}

void Light::Update() {
//#ifndef NDEBUG
//
//	ImGui::SetNextWindowPos(ImVec2{ 0.0f, 18.0f * 2 }, ImGuiCond_FirstUseEver);
//	ImGui::SetNextWindowSize(ImVec2{ 300.0f, float(WinApp::GetInstance()->GetkClientHeight()) - 18.0f * 2 }, ImGuiCond_FirstUseEver);
//	ImGui::Begin("Light");
//	if (ImGui::TreeNode("DirectionalLight / 太陽")) {
//		ImGui::SliderFloat("Intensity / 輝度", &directionalLightData->intensity, 0.0f, 1.0f);
//		ImGui::ColorEdit4("Color / 色", &directionalLightData->color.x);
//		ImGui::SliderFloat3("Direction / 方向", &directionalLightData->direction.x, -1.0f, 1.0f);
//		ImGui::ColorEdit3("SpecularColor / 反射色", &directionalLightData->specularColor.x);
//		ImGui::TreePop();
//	}
//	if (ImGui::TreeNode("PointLight / 点光源")) {
//		ImGui::SliderFloat("Intensity / 輝度", &pointLightData->intensity, 0.0f, 1.0f);
//		ImGui::ColorEdit4("Color / 色", &pointLightData->color.x);
//		ImGui::DragFloat3("Position / 位置", &pointLightData->position.x, 0.1f);
//		ImGui::DragFloat("Radius / ライトの距離", &pointLightData->radius, 1.0f);
//		ImGui::DragFloat("Dacay / ライトの減衰", &pointLightData->dacay, 0.1f);
//		ImGui::ColorEdit3("SpecularColor / 反射色", &pointLightData->specularColor.x);
//		ImGui::TreePop();
//	}
//	if (ImGui::TreeNode("SpotLight / スポットライト")) {
//		ImGui::DragFloat("Intensity / 輝度", &spotLightData->intensity, 0.1f);
//		ImGui::ColorEdit4("Color / 色", &spotLightData->color.x);
//		ImGui::DragFloat3("Position / 位置", &spotLightData->position.x, 0.1f);
//		ImGui::SliderFloat3("Direction / 方向", &spotLightData->direction.x, -1.0f, 1.0f);
//		ImGui::DragFloat("Distance / ライトの距離", &spotLightData->distance, 0.1f);
//		ImGui::DragFloat("Dacay / ライトの減衰", &spotLightData->dacay, 0.1f);
//		ImGui::DragFloat("CosAngle / 余弦", &spotLightData->cosAngle, 0.1f);
//		ImGui::DragFloat("cosFalloffStart / falloff開始の角度", &spotLightData->cosFalloffStart, 0.1f);
//		ImGui::ColorEdit3("SpecularColor / 反射色", &spotLightData->specularColor.x);
//		ImGui::TreePop();
//	}
//	if (ImGui::TreeNode("Scan / スキャン"))
//	{
//		ImGui::ColorEdit3("Color", &scanData->color.x);
//		ImGui::DragFloat("width", &scanData->width, 0.1f);
//		ImGui::DragFloat("radius", &scanData->radius, 0.1f);
//		ImGui::TreePop();
//	}
//	ImGui::End();
//
//#endif // _DEBUG


	spotLightData->cosFalloffStart = max(spotLightData->cosFalloffStart, spotLightData->cosAngle);
	spotLightData->dacay = max(0.0f, spotLightData->dacay);
	spotLightData->cosAngle = max(0.0f, spotLightData->cosAngle);
}
