#include "SceneFadeManager.h"
#include "kMath.h"
#include "TextureManager.h"

SceneFadeManager* SceneFadeManager::instance = nullptr;

SceneFadeManager* SceneFadeManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneFadeManager;
	}
	return instance;
}

void SceneFadeManager::Finalize() {
	delete sprite_;
	delete instance;
	instance = nullptr;
}

void SceneFadeManager::Initialize(const Vector3 color) {
	color_ = color;
	sprite_ = new Sprite();
	sprite_->Initialize("Resources/Sprite/white1x1.png");
	sprite_->SetColor({ color_.x, color_.y, color_.z, alpha_ });
	sprite_->SetScale({ float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()) });
}

void SceneFadeManager::Update() {
	if (fade_)
	{
		fadeTimer_ += 1.0f / 60.0f / fadeTime_;
		alpha_ = Lerp(alphaPre_, goalAlpha_, fadeTimer_);
		if (fadeTimer_ >= 1.0f)
		{
			fade_ = false;
			completeFade_ = true;
			fadeTimer_ = 0.0f;
		}
		sprite_->SetColor({ color_.x, color_.y, color_.z, alpha_ });
	}
	else if (!fade_ && completeFade_)
	{
		fadeTimer_ += 1.0f;
		if (fadeTimer_ > 2.0f)
		{
			fadeTimer_ = 0.0f;
			completeFade_ = false;
		}
	}
	sprite_->Update();
}


void SceneFadeManager::FadeOut(const float time) {
	if (!fade_)
	{
		completeFade_ = false;
		fade_ = true;
		alphaPre_ = alpha_;
		goalAlpha_ = 1.0f;
		fadeTime_ = time;
		fadeTimer_ = 0.0f;
	}
}

void SceneFadeManager::FadeIn(const float time) {
	if (!fade_)
	{
		completeFade_ = false;
		fade_ = true;
		alphaPre_ = alpha_;
		goalAlpha_ = 0.0f;
		fadeTime_ = time;
		fadeTimer_ = 0.0f;
	}
}

void SceneFadeManager::Draw() {
	SpriteBase::GetInstance()->ShaderDraw();
	sprite_->Draw();
}
