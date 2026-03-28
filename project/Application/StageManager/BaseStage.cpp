#include "BaseStage.h"

void BaseStage::Initialize(Player* player, Camera* camera, MouseCursor* mouseCursor){}

std::string BaseStage::GetJsonName()
{
    return std::string();
}

void BaseStage::Update(){}

void BaseStage::DrawObject3d(){}

void BaseStage::DrawFrontSprite(){}

void BaseStage::DrawBackSprite(){}

void BaseStage::Finalize(){}
