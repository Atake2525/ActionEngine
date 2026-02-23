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
#include <memory>

#pragma once

// タイトルシーン
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

	std::unique_ptr<Camera> camera;
	Input* input = nullptr;

	bool start = false;

	std::unique_ptr<Object3d> bootScreen = nullptr;

	std::unique_ptr<Object3d> charModel = nullptr;

	enum class Select {
		Play = 0,
		Setting = 1,
		Exit = 2,
		Credit = 3,
	};
	int maxSelectNum = 3;

	Select select = Select::Play;
	Select selectPre = Select::Play;

	std::unique_ptr<Sprite> gamePad = nullptr;

	std::unique_ptr<Sprite> credit_sound = nullptr;

	bool start_ = false;

};

