#include "StageSelectScene.h"
#include "TextureManager.h"

using namespace std;

void StageSelectScene::Initialize() {
	camera = make_unique<Camera>();
	
	TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

	SkyBox::GetInstance()->SetCamera(camera.get());
	SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

}

void StageSelectScene::Finalize()
{

}

void StageSelectScene::Update()
{

}

void StageSelectScene::Draw()
{

}
