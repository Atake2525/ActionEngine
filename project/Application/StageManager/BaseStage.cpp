#include "BaseStage.h"

void BaseStage::SetContext(AppContext& context) {
	// contextはゲームに一つ。作り直すことが無いので入っていたら代入しない
	if (m_pContext == nullptr)
	{
		m_pContext = &context;
	}
}

void BaseStage::Initialize(Player* player, Camera* camera){}

std::string BaseStage::GetJsonName()
{
    return std::string();
}

void BaseStage::Update(){}

void BaseStage::DrawObject3d(){}

void BaseStage::DrawFrontSprite(){}

void BaseStage::DrawBackSprite(){}

void BaseStage::Finalize(){}
