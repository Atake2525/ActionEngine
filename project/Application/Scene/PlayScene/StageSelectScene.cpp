#include "StageSelectScene.h"
#include "TextureManager.h"
#include "StageCount.h"
#include "FadeManager.h"
#include "GameTime.h"

using namespace std;
using namespace ActionEngine::Stage;

void StageSelectScene::Initialize() {
	camera = make_unique<Camera>();
	
	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

	selectFrame = make_unique<UI>();

	GameTime::GetInstance()->SetDeltaPoint();
	FadeManager::GetInstance()->FadeIn(1.0f);
}

void StageSelectScene::Finalize()
{

}

void StageSelectScene::Update()
{
	if (!start_)
	{
		if (FadeManager::GetInstance()->CompleteFade())
		{
			start_ = true;
		}
		else
		{
			return;
		}
	}


}

void StageSelectScene::Draw()
{

}
