
#include "FadeManager.h"
#include "kMath.h"
#include "TextureManager.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "EasingUtility.h"

void FadeManager::Initialize(WinApp& winApp, Render2DBase& render2DBase, GameTime& gameTime) {
	m_pWinApp = &winApp;
	m_pRender2DBase = &render2DBase;
	m_pGameTime = &gameTime;
	color_ = Vector3::Zero;
	m_sprite.Initialize("Resources/Sprite/white1x1.png");
	m_sprite.SetColor({ color_.x, color_.y, color_.z, alpha_ });
	m_sprite.SetScale({ float(m_pWinApp->GetkClientWidth()), float(m_pWinApp->GetkClientHeight()) });
	fadeFunction = [this]() {
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
	float deltaTime = m_pGameTime->GetDeltaTime();
	if (completeFade_)
	{
		completeFade_ = false;
	}
	if (fade_)
	{
		fadeTimer_ += deltaTime / fadeTime_;
		alpha_ = std::clamp(alpha_, 0.0f, 1.0f);
		if (fadeFunction)
		{
			fadeFunction();
		}
		if (fadeTimer_ >= 1.0f)
		{
			fade_ = false;
			completeFade_ = true;
			fadeTimer_ = 0.0f;
			if (finishedFadeFunction)
			{
				finishedFadeFunction();
				finishedFadeFunction = nullptr;
			}
		}
		m_sprite.SetColor({ color_.x, color_.y, color_.z, alpha_ });
	}
	m_sprite.Update();


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
	if (ImGui::Button("FadeIn"))
	{
		FadeIn(1.0f);
	}
	if (ImGui::Button("FadeOut"))
	{
		FadeOut(1.0f);
	}

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
		fadeFunction = [this]() {
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

		fadeFunction = [this]() {
			alpha_ = EaseInBack(alphaPre_, goalAlpha_, fadeTimer_);
			};
	}
}

void FadeManager::Draw() {
	m_pRender2DBase->ShaderDraw();
	m_sprite.Draw();
}
