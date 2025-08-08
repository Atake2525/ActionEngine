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

	Object3d* grid = nullptr;

	Object3d* land = nullptr;

	Object3d* terrain = nullptr;

	Object3d* box = nullptr;

	Camera* camera = nullptr;
	Object3d* cameraObject = nullptr;

	bool finished = false;

	Transform cameraTransform;
	Transform modelTransform;

	Input* input = nullptr;

	AABB aabb;

	bool enableLighting = true;

	bool cursorshow = true;

	//Sprite* sprite = nullptr;


	Vector2 leftTop;
	Transform transformSprite;

	Player* player_ = nullptr;
};

