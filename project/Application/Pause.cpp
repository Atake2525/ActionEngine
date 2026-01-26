#include "Pause.h"
#include "WinApp.h"
#include "Input.h"

using namespace std;

void Pause::Initialize() {
	float windowXSize = float(WinApp::GetInstance()->GetkClientWidth());

	std::array<std::unique_ptr<Sprite>, 5> sprites;
	for (int i = 0; i < sprites.size(); i++)
	{
		sprites[i] = make_unique<Sprite>();
		sprites[i]->Initialize("Resources/Sprite/Pause/pause.png");
	}
	sprites[1]->Initialize("Resources/Sprite/Pause/restart.png");
	sprites[2]->Initialize("Resources/Sprite/Pause/setting.png");
	sprites[3]->Initialize("Resources/Sprite/Pause/stageselect.png");
	sprites[4]->Initialize("Resources/Sprite/Pause/title.png");
	for (int i = 0; i < 4; i++)
	{
		sprites[i]->SetScale({ 0.5f, 0.5f });
		sprites[i]->SetAnchorPoint({ 0.5f, 0.5f });
		sprites[i]->SetPosition({ 0.0f, sprites[i]->GetTextureSize().y * i });
	}
}

void Pause::Update() {
	if (Input::GetInstance()->TriggerKeyInt(DIK_ESCAPE))
	{
		pause = !pause;
	}
	for (int i = 0; i < 5; i++)
	{
		//sprites[i]->Update();
	}
}

void Pause::Draw() {
	if (pause)
	{
		for (int i = 0; i < 5; i++)
		{
			//sprites[i]->Draw();
		}
	}
}
