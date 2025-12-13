#include "BaseScene.h"
#include "Camera.h"
#include "Input.h"
#include "SkyBox.h"
#include <memory>
#pragma once

class StageSelectScene : public BaseScene
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	const bool& EndRequest() override { return finished; }


private:
	bool finished = false;
	std::unique_ptr<Camera> camera;
	Input* input = nullptr;
};

