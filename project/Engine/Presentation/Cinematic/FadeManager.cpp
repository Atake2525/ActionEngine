#include "FadeManager.h"
#include "kMath.h"
#include "TextureManager.h"
#include "GameTime.h"
#include "ImGuiManager.h"

FadeManager* FadeManager::instance = nullptr;

FadeManager* FadeManager::GetInstance() {
	if (instance == nullptr) {
		instance = new FadeManager;
	}
	return instance;
}

void FadeManager::Finalize() {
	delete sprite_;
	delete instance;
	instance = nullptr;
}

void FadeManager::Initialize(const Vector3 color) {
	color_ = color;
	sprite_ = new Sprite();
	sprite_->Initialize("Resources/Sprite/white1x1.png");
	sprite_->SetColor({ color_.x, color_.y, color_.z, alpha_ });
	sprite_->SetScale({ float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()) });
}

const bool& FadeManager::CompleteFade()
{
	bool complete = completeFade_;
	completeFade_ = false;
	return complete;
	// TODO: return ステートメントをここに挿入します
}

void FadeManager::Update() {
	float deltaTime = GameTime::GetInstance()->GetDeltaTime();
	if (fade_)
	{
		fadeTimer_ += 1.0f / 60.0f / fadeTime_;
		alpha_ = Lerp(alphaPre_, goalAlpha_, fadeTimer_);
		alpha_ = std::clamp(alpha_, 0.0f, 1.0f);
		if (fadeTimer_ >= 1.0f)
		{
			fade_ = false;
			completeFade_ = true;
			fadeTimer_ = 0.0f;
		}
		sprite_->SetColor({ color_.x, color_.y, color_.z, alpha_ });
	}
	sprite_->Update();

#ifndef NDEBUG
	ImGui::Begin("FadeInOut");
	ImGui::SliderFloat("fadeTimer", &fadeTimer_, 0.0f, 1.0f);
	ImGui::DragFloat("alpha", &alpha_);
	ImGui::DragFloat("DeltaTime", &deltaTime);
	ImGui::End();
#endif // !NDEBUG



}


void FadeManager::FadeOut(const float time) {
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

void FadeManager::FadeIn(const float time) {
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

void FadeManager::Draw() {
	SpriteBase::GetInstance()->ShaderDraw();
	sprite_->Draw();
}
