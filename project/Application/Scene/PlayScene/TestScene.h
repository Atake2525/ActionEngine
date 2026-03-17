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
#include "ActionPlayer.h"
#include "Player.h"
#include "JsonLoader.h"
#include "Trap.h"
#include "Goal.h"
#include "BaseStage.h"
#include "TutorialStage.h"
#include "DebugLine.h"
#include "Pause.h"
#include "PlayerUI.h"
#include <thread>
#include <functional>

#pragma once

// テスト用シーン
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

	std::unique_ptr<ActionPlayer> actionPlayer;

	std::unique_ptr<Object3d> box;
	std::vector<std::unique_ptr<Object3d>> boxes;

	std::unique_ptr<BaseStage> stage;

    std::unique_ptr<Player> player;

	std::unique_ptr<PlayerUI> playerUi;
	std::unique_ptr<Pause> pause;

	std::unique_ptr<Sprite> sprite;
	float tim = 0.0f;

	bool start_ = false;
};

