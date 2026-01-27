#include "Pause.h"
#include "WinApp.h"
#include "Input.h"
#include "EasingUtility.h"
#include "GameTime.h"

using namespace std;

void Pause::Initialize() {
	float windowXSize = float(WinApp::GetInstance()->GetkClientWidth());

	for (int i = 0; i < sprites.size(); i++)
	{
		sprites[i] = make_unique<Sprite>();
		sprites[i]->Initialize("Resources/Sprite/Pause/pause.png");
		//sprites[i]->SetScale({ sprites[i]->GetScale() * 0.5f });
		sprites[i]->SetAnchorPoint({ 0.0f, 0.0f });
		sprites[i]->SetPosition({ 0.0f, i * 20.0f });
	}
}

void Pause::Update() {
	if (Input::GetInstance()->TriggerKeyInt(DIK_ESCAPE) && !m_pauseAnim)
	{
		m_pauseAnim = !m_pauseAnim;
	}

	if (m_pauseAnim)
	{
		for (int i = 0; i < 5; i++)
		{
			if (!m_pause)
			{

			}
			sprites[i]->Update();
		}
	}

	for (int i = 0; i < 5; i++)
	{
		sprites[i]->Update();
	}
}

void Pause::Draw() {
	if (m_pause)
	{
		for (int i = 0; i < 5; i++)
		{
			sprites[i]->Draw();
		}
	}
}
