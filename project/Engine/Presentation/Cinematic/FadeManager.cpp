
#include "FadeManager.h"
#include "kMath.h"
#include "TextureManager.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "EasingUtility.h"


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
	function = [this]() {
		alpha_ = Lerp(alphaPre_, goalAlpha_, fadeTimer_);
		};
}

const bool FadeManager::CompleteFade()
{
	bool complete = completeFade_;
	completeFade_ = false;
	/*if (!complete)
	{
		complete = true;
	}*/
	return complete;
	// TODO: return ステートメントをここに挿入します
}

void FadeManager::Update() {
	float deltaTime = GameTime::GetInstance()->GetDeltaTime();
	if (completeFade_)
	{
		completeFade_ = false;
	}
	if (fade_)
	{
		fadeTimer_ += deltaTime / fadeTime_;
		if (function)
		{
			function();
		}
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
	maxDeltaTime_ = std::max(maxDeltaTime_, deltaTime);
	ImGui::Begin("FadeInOut");
	ImGui::SliderFloat("fadeTimer", &fadeTimer_, 0.0f, 1.0f);
	ImGui::DragFloat("alpha", &alpha_, 0.01f);
	ImGui::DragFloat("DeltaTime", &deltaTime);
	ImGui::Text("MaxDeltaTime: %.6f", maxDeltaTime_);
	if (ImGui::Button("ResetMaxDeltaTime"))
	{
		maxDeltaTime_ = 0.0f;
	}
	ImGui::End();

	sprite_->SetColor({ color_.x, color_.y, color_.z, alpha_ });

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
		function = [this]() {
			alpha_ = Lerp(alphaPre_, goalAlpha_, fadeTimer_);
			};
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

		function = [this]() {
			alpha_ = EaseInBack(fadeTimer_, alphaPre_, goalAlpha_);
			};
	}
}

void FadeManager::Draw() {
	SpriteBase::GetInstance()->ShaderDraw();
	sprite_->Draw();
}
