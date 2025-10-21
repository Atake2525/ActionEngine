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
#include "Player.h"
#include "SkyBox.h"
#include "CollisionManager.h"
#include "Goal.h"
#include <memory>

#pragma once

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

	const bool& EndRequest() override { return finished; }

private:
	float speed = 0.25f;

	bool sneak = false;

	std::unique_ptr<Object3d> land = nullptr;
	std::unique_ptr<Object3d> floor = nullptr;

	std::unique_ptr<Camera> camera = nullptr;

	bool finished = false;

	Transform cameraTransform;
	Transform modelTransform;

	Input* input = nullptr;

	AABB aabb;

	bool enableLighting = true;

	bool cursorshow = true;

	Vector2 leftTop;
	Transform transformSprite;

	std::unique_ptr<Player> player_ = nullptr;

	bool isGoal_ = false;

	bool start_ = false;

	bool startMovie_ = false;
	float movieTimer_ = 0.0f;
	float movieTime_ = 2.0f;
	int phase_ = 0;

	std::unique_ptr<Goal> goal_ = nullptr;
};

