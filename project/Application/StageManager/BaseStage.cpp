#include "BaseStage.h"

void BaseStage::Initialize(Player* player, Camera* camera){}

std::string BaseStage::GetJsonName()
{
    return std::string();
}

LevelEditor& BaseStage::GetLevelEditor()
{
    return m_levelEditor;
}

const LevelEditor& BaseStage::GetLevelEditor() const
{
    return m_levelEditor;
}

void BaseStage::Update(){}

void BaseStage::DrawObject3d(){}

void BaseStage::DrawFrontSprite(){}

void BaseStage::DrawBackSprite(){}

void BaseStage::Finalize(){}
