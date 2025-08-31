#include "Goal.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "FadeManager.h"


Goal::~Goal()
{
	delete clearBackScreenSprite_;
	delete clearTextSprite_;
}

void Goal::Initalize() {
	clearBackScreenSprite_ = new Sprite();
	clearBackScreenSprite_->Initialize("Resources/Sprite/UI/stageClearBackScreen.png");

	clearTextSprite_ = new Sprite();
	clearTextSprite_->Initialize("Resources/Sprite/UI/stageClearText.png");

	input = Input::GetInstance();
}

void Goal::Update() {
	clearBackScreenSprite_->Update();
	clearTextSprite_->Update();

	if (input->TriggerKey(DIK_SPACE) || input->TriggerKey(DIK_RETURN) || input->TriggerMouse(0))
	{
		SceneManager::GetInstance()->SetNextScene("TITLE");
		//FadeManager::GetInstance()->FadeOut(1.0f);
	}

}

void Goal::Draw() {

	clearBackScreenSprite_->Draw();
	clearTextSprite_->Draw();

}