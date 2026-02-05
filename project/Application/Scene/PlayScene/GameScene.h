#include "BaseScene.h"
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
#include "SceneManager.h"
#include "SkyBox.h"
#include "CollisionManager.h"
#include <memory>
#include "GameOver.h"
#include "GameClearScene.h"
#include "ActionPlayer.h"
#include "BaseStage.h"

#pragma once

// ゲームシーン
class GameScene : public BaseScene
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

	// 終了処理
	const bool& EndRequest() override { return finished; }

private:
	float speed = 0.25f;

	bool sneak = false;

	std::unique_ptr<Camera> camera = nullptr;

	bool finished = false;

	Transform cameraTransform;
	Transform modelTransform;

	Input* input = nullptr;

	AABB aabb;

	bool enableLighting = true;

	bool cursorshow = false;

	Vector2 leftTop;
	Transform transformSprite;

	std::unique_ptr<ActionPlayer> player_ = nullptr;

	bool start_ = false;

	bool startMovie_ = false;
	float movieTimer_ = 0.0f;
	float movieTime_ = 2.0f;
	int phase_ = 0;

	bool back = false;

	std::unique_ptr<GameOver> gameOver_ = nullptr;
	std::unique_ptr<GameClearScene> gameClear_ = nullptr;

	struct Tutorial
	{
        std::unique_ptr<Sprite> sprite;
		bool isClear = false;
        float timer = 0.0f;
		Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	std::unique_ptr<BaseStage> stage;

	std::unique_ptr<Object3d> stageObject;

};

