#include "DemoScene.h"
#include "Light.h"
#include "GameTime.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include <cmath>

using namespace std;

void DemoScene::Initialize() {

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = make_unique<Camera>();
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 2.8f, -8.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/Debug/dds/sky.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/Debug/dds/sky.dds");

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());

	struct PBRTestMaterial
	{
		const char* fileName;
		Vector3 translate;
		Vector4 color;
		float metallic;
		float roughness;
	};

	const PBRTestMaterial testMaterials[] = {
		{ "PBR_DielectricRough.obj", { -3.6f, 0.0f, 0.0f }, { 0.95f, 0.18f, 0.12f, 1.0f }, 0.0f, 0.85f },
		{ "PBR_DielectricSmooth.obj", { -1.2f, 0.0f, 0.0f }, { 0.95f, 0.18f, 0.12f, 1.0f }, 0.0f, 0.18f },
		{ "PBR_MetalRough.obj", { 1.2f, 0.0f, 0.0f }, { 0.9f, 0.82f, 0.62f, 1.0f }, 1.0f, 0.65f },
		{ "PBR_MetalSmooth.obj", { 3.6f, 0.0f, 0.0f }, { 0.9f, 0.82f, 0.62f, 1.0f }, 1.0f, 0.12f },
	};

	pbrObjects.reserve(_countof(testMaterials));
	for (const PBRTestMaterial& material : testMaterials)
	{
		auto object = make_unique<Object3d>();
		object->Initialize();
		object->SetModel("Resources/Debug/obj", material.fileName, true, false);
		object->SetTranslate(material.translate);
		object->SetScale({ 0.8f, 0.8f, 0.8f });
		object->SetColor(material.color);
		object->SetPBRMaterial(material.metallic, material.roughness);
		pbrObjects.push_back(std::move(object));
	}

	struct NormalMapCompareModel
	{
		const char* fileName;
		Vector3 translate;
	};

	const NormalMapCompareModel normalMapCompareModels[] = {
		{ "PBR_NormalMap_OFF.obj", { -1.45f, 2.0f, 1.25f } },
		{ "PBR_NormalMap_ON.obj", { 1.45f, 2.0f, 1.25f } },
	};

	normalMapObjects.reserve(_countof(normalMapCompareModels));
	for (const NormalMapCompareModel& model : normalMapCompareModels)
	{
		auto object = make_unique<Object3d>();
		object->Initialize();
		object->SetModel("Resources/Debug/obj", model.fileName, true, false);
		object->SetTranslate(model.translate);
		object->SetScale({ 1.2f, 1.2f, 0.12f });
		object->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		object->SetPBRMaterial(0.0f, 0.55f);
		normalMapObjects.push_back(std::move(object));
	}

	pointLightMarker = make_unique<Object3d>();
	pointLightMarker->Initialize();
	pointLightMarker->SetModel("Resources/Debug/obj", "PBR_LightMarker.obj", false, false);
	pointLightMarker->SetScale({ 0.16f, 0.16f, 0.16f });
	pointLightMarker->SetColor({ 1.0f, 0.82f, 0.25f, 1.0f });

	Light::GetInstance()->SetIntensityDirectionalLight(0.12f);
	Light::GetInstance()->SetDirectionDirectionalLight(Normalize({ 0.25f, -1.0f, 0.15f }));
	Light::GetInstance()->SetIntensityPointLight(1.0f);
	Light::GetInstance()->SetRadiusPointLight(7.5f);
	Light::GetInstance()->SetdacayPointLight(1.6f);
	Light::GetInstance()->SetColorPointLight({ 1.0f, 0.86f, 0.65f, 1.0f });
	Light::GetInstance()->SetIntensitySpotLight(0.0f);

}

void DemoScene::Update() {

	camera->Update();
	pointLightAngle += GameTime::GetInstance()->GetDeltaTime() * 1.2f;

	const Vector3 lightPosition = {
		std::cos(pointLightAngle) * 3.2f,
		1.9f,
		std::sin(pointLightAngle) * 3.2f
	};
	Light::GetInstance()->SetPositionPointLight(lightPosition);

	for (auto& object : pbrObjects)
	{
		Vector3 rotate = object->GetRotate();
		rotate.y += GameTime::GetInstance()->GetDeltaTime() * 0.35f;
		object->SetRotate(rotate);
		object->Update();
	}

	for (auto& object : normalMapObjects)
	{
		object->Update();
	}

#ifndef NDEBUG
	ImGui::Begin("PBR Test");
	ImGui::Text("Top: normal map OFF / ON");
	ImGui::Text("Center: base PBR roughness and metallic");
	ImGui::End();
#endif

	if (pointLightMarker)
	{
		pointLightMarker->SetTranslate(lightPosition);
		pointLightMarker->Update();
	}

	if (input->TriggerKey(DIK_RETURN))
	{
		SceneManager::GetInstance()->SetNextScene("GAMESCENE");
	}

	SkyBox::GetInstance()->Update();

	input->Update();
}

void DemoScene::Draw() {

	Render2DBase::GetInstance()->ShaderDraw();


	Object3dBase::GetInstance()->ShaderDraw();

	for (auto& object : pbrObjects)
	{
		object->Draw();
	}

	for (auto& object : normalMapObjects)
	{
		object->Draw();
	}

	if (pointLightMarker)
	{
		pointLightMarker->Draw();
	}

	SkinningObject3dBase::GetInstance()->ShaderDraw();

	WireFrameObjectBase::GetInstance()->ShaderDraw();

	ParticleManager::GetInstance()->Draw();

}

void DemoScene::Finalize() {
}
