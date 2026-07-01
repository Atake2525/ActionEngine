#include "MyGame.h"

void MyGame::Initialize() {
    FrameWork::Initialize();
}

void MyGame::Update() {
    FrameWork::Update();
    finished = FrameWork::LoopOut();
}

void MyGame::Draw() {

//    // ImGuiの内部コマンドを生成する
//    ImGui::Render();
//
//
//    SkyBox::GetInstance()->Draw();
//    SceneManager::GetInstance()->Draw();
//    FadeManager::GetInstance()->Draw();
//
//    m_directXBase->PostDrawRenderTexture();
//
//    m_directXBase->PreDraw();
//
//    ParticleManager::GetInstance()->Draw();
//
//    m_directXBase->ApplyFullViewport();
//
//#ifndef NDEBUG
//    // 実際のcommandListのImGuiの描画コマンドを積む
//    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_directXBase->GetCommandList().Get());
//#endif // !NDEBUG
//
//
//    m_directXBase->PostDraw();
//
//    SceneManager::GetInstance()->CallStart();
}

void MyGame::Finalize() {

    /*WinApp::GetInstance()->Finalize();

    SrvManager::GetInstance()->Finalize();

    ImGuiManager::GetInstance()->Finalize();

    Render2DBase::GetInstance()->Finalize();

    Object3dBase::GetInstance()->Finalize();

    DebugLineBase::GetInstance()->Finalize();

    SkyBox::GetInstance()->Finalize();

    ModelBase::GetInstance()->Finalize();

    TextureManager::GetInstance()->Finalize();

    ModelManager::GetInstance()->Finalize();

    ParticleManager::GetInstance()->Finalize();

    CollisionManager::GetInstance()->Finalize();

    JsonLoader::GetInstance()->Finalize();

    Light::GetInstance()->Finalize();

    Input::GetInstance()->Finalize();

    Audio::GetInstance()->Finalize();

    GameTime::GetInstance()->Finalize();

    FadeManager::GetInstance()->Finalize();*/

    FrameWork::Finalize();
}
