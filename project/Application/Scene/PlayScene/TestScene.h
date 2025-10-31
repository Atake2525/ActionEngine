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
#include <wrl.h>
#include <memory>
#include "Player.h"
#include "JsonLoader.h"
#include "GameOver.h"
#include "Trap.h"

#pragma once

class TestScene : public BaseScene
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

	bool cursorshow = true;

	//std::unique_ptr<Object3d> box1;
	std::unique_ptr<Object3d> box2;
	std::unique_ptr<Object3d> plate;

	std::unique_ptr<GameOver> gameOverSprite;

	std::unique_ptr<Player> player;

	Object3d* grid = nullptr;

	std::unique_ptr<Trap> trap;
};

