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
    engine.platform.input.Initialize(engine.platform.window);
    

    engine.graphics.dx.Initialize();
    engine.graphics.srv.Initialize(engine.graphics.dx);
    engine.graphics.offScreen.Initialize(engine.graphics.dx, engine.graphics.srv, engine.platform.window);
    engine.graphics.render2DBase.Initialize(engine.graphics.dx);
    engine.graphics.object3DBase.Initialize(engine.graphics.dx);

    engine.platform.time.Initialize(engine.graphics.dx);
#ifndef NDEBUG
    engine.graphics.imgui.Initialize(engine.graphics.dx, engine.graphics.srv, engine.platform.window);
    engine.graphics.debugLine.Initialize(engine.graphics.dx);
#endif // !NDEBUG

    engine.assets.audio.Initialize();
    engine.assets.textures.Initialize(engine.graphics.dx, engine.graphics.srv);
    engine.assets.json.Initialize();

    engine.presentation.fade.Initialize(engine.platform.window, engine.graphics.render2DBase, engine.platform.time);
}

void Runtime::SetupWorld() {
    WorldContext& world = m_context.world;
    EngineContext& engine = m_context.engine;

    world.light.Initialize(engine.graphics.dx);
    world.collision.Initialize();
    world.particles.Initialize(engine.graphics.dx, engine.graphics.srv , engine.assets.textures);
    world.skyBox.Initialize(engine.graphics.dx, engine.graphics.srv, engine.assets.textures);
}

void Runtime::SetupGame() {
    m_context.game.sceneManager.SetContext(m_context);
}
