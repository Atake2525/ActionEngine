#include "DebugLab.h"
#include "Runtime.h"

void DebugLab::SetContext(AppContext& context) {
    BaseStage::SetContext(context);
}

void DebugLab::Initialize(Player* player, Camera* camera) {
    m_player = player;
    m_camera = camera;

    EngineContext& enContext = m_pContext->engine;

    m_pStageObject = 
}

std::string DebugLab::GetJsonName() {
    return "DebugLab";
}

void DebugLab::Update() {

}

void DebugLab::DrawBackSprite() {

}

void DebugLab::DrawObject3d() {

}

void DebugLab::DrawFrontSprite() {

}

void DebugLab::Finalize() {

}