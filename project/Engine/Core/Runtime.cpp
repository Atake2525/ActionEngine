#include "Runtime.h"

void Runtime::Initialize() {
    SetupEngine();
    SetupWorld();
    SetupGame();
}

void Runtime::Update() {
    if (m_context.engine.platform.window.ProcessMessage()) {
        m_loopOut = true;
        return;
    }

    m_context.engine.platform.time.Update();
    m_context.engine.platform.input.Update();

#ifndef NDEBUG
    m_context.engine.graphics.imgui.Update();
    m_context.engine.platform.time.DrawImGui();
    ImGui::Begin("Performance");
    float timeScale = m_context.engine.platform.time.GetTimeScale();
    ImGui::DragFloat("TimeScale", &timeScale, 0.01f, 0.0f, 10.0f);
    m_context.engine.platform.time.SetTimeScale(timeScale);
    float framePerSecond = 1.0f / m_context.engine.platform.time.GetDeltaTime();
    ImGui::Text("FPS: %.1f", framePerSecond);
    ImGui::End();

    float fps = 1.0f / m_context.engine.platform.time.GetDeltaTime();
    ImGui::Begin("performance");
    if (fps > 50)
    {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %.1f", fps);
    }
    else if (fps > 30)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %.1f", fps);
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "FPS: %.1f", fps);
    }
    ImGui::End();

#endif // !NDEBUG

    m_context.world.light.Update();
    m_context.game.sceneManager.Update();
    m_context.world.particles.Update();
    m_context.engine.assets.audio.Update();
    m_context.engine.presentation.fade.Update();

    if (m_context.game.sceneManager.EndRequest()) {
        m_loopOut = true;
    }
}

void Runtime::Draw() {

    ImGui::Render();

    m_context.engine.graphics.dx.BeginOffScreenRendering(m_context.engine.graphics.offScreen.GetRenderTextureResource(), m_context.engine.graphics.offScreen.GetRenderTargetDescriptorHandle());

    m_context.engine.graphics.srv.PreDraw();
    m_context.engine.graphics.dx.ApplyFullViewport();
    m_context.world.skyBox.Draw();
    m_context.game.sceneManager.Draw();
    m_context.engine.presentation.fade.Draw();
    m_context.world.particles.Draw();

    m_context.engine.graphics.dx.EndOffScreenRendering(m_context.engine.graphics.offScreen.GetRenderTextureResource());

    m_context.engine.graphics.dx.PreDraw();
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

#ifndef NDEBUG
    engine.platform.window.Initialize();
#else
    engine.platform.window.Initialize(1280, 720, WindowMode::Window, L"走快");
#endif // !NDEBUG

    engine.platform.input.Initialize(engine.platform.window);
    

    engine.graphics.dx.Initialize(engine.platform.window);
    engine.graphics.srv.Initialize(engine.graphics.dx);
    engine.assets.textures.Initialize(engine.graphics.dx, engine.graphics.srv);
    engine.graphics.offScreen.Initialize(engine.graphics.dx, engine.graphics.srv, engine.platform.window, engine.assets.textures);
    engine.graphics.render2DBase.Initialize(engine.graphics.dx);
    engine.graphics.object3DBase.Initialize(engine.graphics.dx, m_context.world.light);

    engine.platform.time.Initialize(engine.graphics.dx);
    engine.graphics.imgui.Initialize(engine.graphics.dx, engine.graphics.srv, engine.platform.window);
#ifndef NDEBUG
    engine.graphics.debugLine.Initialize(engine.graphics.dx);
#endif // !NDEBUG

    engine.assets.audio.Initialize();
    engine.assets.json.Initialize();

    engine.presentation.fade.Initialize(engine.platform.window, engine.graphics.render2DBase, engine.platform.time, engine.graphics.dx, engine.graphics.srv, engine.assets.textures);
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
    EngineContext& engine = m_context.engine;
    WorldContext& world = m_context.world;
    GameContext& game = m_context.game;

    engine.assets.models.SetContext(engine.graphics.dx, engine.graphics.srv, engine.assets.textures, engine.graphics.object3DBase, world.skyBox);
    engine.assets.models.Initialize();

    game.object3dFactory.SetContext(engine.graphics.dx, engine.graphics.srv, engine.graphics.object3DBase, engine.platform.time);
    game.spriteFactory.SetContext(engine.graphics.dx, engine.graphics.srv, engine.assets.textures, engine.platform.window);

    game.sceneFactory.SetContext(m_context);
    game.sceneManager.SetContext(m_context);
    game.stageCount.Initialize();

    engine.assets.audio.LoadMP3("Resources/sound/select.mp3", "select");
    engine.assets.audio.LoadMP3("Resources/sound/enter.mp3", "select_enter");
    engine.assets.audio.LoadMP3("Resources/sound/cancel.mp3", "select_cancel");

    game.sceneManager.GetSettingManager().SetContext(engine.assets.json);
    game.sceneManager.GetSettingManager().Load("KeyConfig.json", Setting::SettingType::KeyConfig);
    game.sceneManager.GetSettingManager().Load("Audio.json", Setting::SettingType::AudioConfig);
    game.sceneManager.SetNextScene("GAMESCENE");

    engine.platform.window.OpenWindow();
}
