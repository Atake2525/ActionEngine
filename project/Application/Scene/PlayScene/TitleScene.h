#include "Object3d.h"
#include "Object3dBase.h"
#include "SpriteBase.h"
#include "Camera.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Input.h"
#include "WireFrameObjectBase.h"
#include "SkinningObject3dBase.h"
#include "Sprite.h"
#include "AABB.h"
#include "ParticleManager.h"
#include "Audio.h"
#include "BaseScene.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include <map>
#include "UI.h"

#pragma once

class TitleScene : public BaseScene
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
	Camera* camera = nullptr;
	Input* input = nullptr;

	bool start = false;

	Object3d* stageModel = nullptr;

	Object3d* playerModel = nullptr;

	enum class Select {
		Play = 0,
		Setting = 1,
		Exit = 2,
		Credit = 3,
	};
	int maxSelectNum = 3;

	Select select = Select::Play;
	Select selectPre = Select::Play;

	UI* startUI = nullptr;

	UI* playUI = nullptr;

	UI* exitUI = nullptr;

	UI* settingUI = nullptr;

	UI* creditUI = nullptr;

	Sprite* uiFrame = nullptr;

	Sprite* gamePad = nullptr;

	Sprite* gamePadOnFrame = nullptr;

	Sprite* credit_sound = nullptr;

	Vector3 uiFrameStartPoint = { 0.0f, 0.0f, 0.0f };

	Vector3 uiFrameEndPoint = { 0.0f, 0.0f, 0.0f };

	float uiFrameMoveTimer = 0.0f;

	float uiFrameMoveLImitTime = 0.6f;

	bool isUIFrameMove = false;

	bool showCredit = false;

};

