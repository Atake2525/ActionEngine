#pragma once

#include "Sprite.h"
#include "Render2DBase.h"
#include <functional>

class FadeManager
{
private:
	static FadeManager* instance;

	// コンストラクタ、デストラクタの隠蔽
	FadeManager() = default;
	~FadeManager() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	FadeManager(FadeManager&) = delete;
	FadeManager& operator=(FadeManager&) = delete;

public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>TextureManager* instance</returns>
	static FadeManager* GetInstance();

	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	const bool CompleteFade();
	const bool& IsFade() const { return fade_; }

	void SetColor(const Vector3& color) { color_ = color; }

	void Update();

	void FadeOut(const float time = 0.0f);

	void FadeIn(const float time = 0.0f);

	void SetFinishedFadeFunction(std::function<void()>& func) { finishedFadeFunction = func; }

	void ClearFunction() { finishedFadeFunction = nullptr; }

	void Draw();

private:
	bool fade_ = false;
	bool completeFade_ = false;

	float fadeTimer_ = 0.0f;
	float fadeTime_ = 0.0f;

	float maxDeltaTime_ = 0.0f;

	float goalAlpha_ = 0.0f;

	float alpha_ = 0.0f;
	float alphaPre_ = 0.0f;

	Vector3 color_ = { 1.0f, 1.0f, 1.0f };

	std::function<void()> fadeFunction;
	std::function<void()> finishedFadeFunction;

	Sprite* sprite_ = nullptr;
};

