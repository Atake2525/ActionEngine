#include "Runtime.h"

void Runtime::Initialize() {
    SetupEngine();
    SetupWorld();
    SetupGame();
}

void Runtime::Update() {
    m_context.engine.platform.time.Update();
    m_context.engine.platform.input.Update();
}

void Runtime::Draw() {

    ImGui::Render();

    m_context.engine.graphics.dx.BeginOffScreenRendering(m_context.engine.graphics.offScreen.GetRenderTextureResource(), m_context.engine.graphics.offScreen.GetRenderTargetDescriptorHandle());

    m_context.world.skyBox.Draw();
    m_context.game.sceneManager.Draw();
    m_context.engine.presentation.fade.Draw();
    m_context.world.particles.Draw();

    m_context.engine.graphics.dx.EndOffScreenRendering(m_context.engine.graphics.offScreen.GetRenderTextureResource());

    m_context.engine.graphics.dx.PreDraw();
    m_context.engine.graphics.srv.PreDraw();
    m_context.engine.graphics.dx.ApplyFullViewport();
    m_context.engine.graphics.offScreen.Draw();

#ifndef NDEBUG
    // 実際のcommandListのImGuiの描画コマンドを積む
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_context.engine.graphics.dx.GetCommandList().Get());
#endif // !NDEBUG


    m_context.engine.graphics.dx.PostDraw();

    m_context.game.sceneManager.CallStart();

}

const bool Runtime::WindowProcessMessage() { 
    return m_context.engine.platform.window.ProcessMessage(); 
}

void Runtime::SetupEngine() {
    EngineContext& engine = m_context.engine;

    engine.platform.window.Initialize();
    engine.platform.input.Initialize();
    engine.platform.time.Initialize();

    engine.graphics.dx.Initialize();
    engine.graphics.srv.Initialize();
    engine.graphics.offScreen.Initialize(&engine.graphics.dx, &engine.graphics.srv);
    engine.graphics.render2DBase.Initialize();
    engine.graphics.object3DBase.Initialize();
#ifndef NDEBUF
    engine.graphics.imgui.Initialize();
    engine.graphics.debugLine.Initialize();
#endif // !NDEBUF

    engine.assets.audio.Initialize();
    engine.assets.textures.Initialize();
    engine.assets.modelBase.Initialize();
    engine.assets.models.Initialize();
    engine.assets.json.Initialize();

    engine.presentation.fade.Initialize();
}

void Runtime::SetupWorld() {
    WorldContext& world = m_context.world;

    world.light.Initialize();
    world.collision.Initialize();
    world.particles.Initialize();
    world.skyBox.Initialize();
}

