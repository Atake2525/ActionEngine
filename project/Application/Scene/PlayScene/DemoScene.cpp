#include "DemoScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "EngineContext.h"

using namespace std;

void DemoScene::Initialize() {
	AppContext& context = *m_pContext;

	//ModelManager::GetInstance()->LoadModel("Resources/Model/gltf/human", "walkMultiMaterial.gltf", true, true);

	camera = make_unique<Camera>(context.engine.platform.window);
	camera->SetRotate(Vector3(SwapRadian(10.0f), 0.0f, 0.0f));
	camera->SetPosition({ 0.0f, 2.8f, -8.0f });

	context.engine.assets.textures.LoadTexture("Resources/white1x1.dds");

	context.world.skyBox.SetCamera(camera.get());
	context.world.skyBox.SetTexture("Resources/white1x1.dds");

	input = &context.engine.platform.input;
	input->ShowMouseCursor(true);

	context.engine.graphics.object3DBase.SetDefaultCamera(camera.get());

	context.world.particles.SetCamera(camera.get());
}

void DemoScene::Update() {

	camera->Update();

	if (input->TriggerKey(DIK_RETURN))
	{
		m_pContext->game.sceneManager.SetNextScene("GAMESCENE");
	}

	m_pContext->world.skyBox.Update(m_pContext->world.light);

	input->Update();
}

void DemoScene::Draw() {

	m_pContext->engine.graphics.render2DBase.ShaderDraw();


	m_pContext->engine.graphics.object3DBase.ShaderDraw();

	m_pContext->world.particles.Draw();

}

void DemoScene::Finalize() {
}
