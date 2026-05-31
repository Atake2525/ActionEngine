#include "Object3d.h"
#include "Object3dBase.h"
#include "Render2DBase.h"
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
#include <memory>
#include <vector>

#pragma once

class DemoScene : public BaseScene
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
	std::vector<std::unique_ptr<Object3d>> pbrObjects;
	std::vector<std::unique_ptr<Object3d>> normalMapObjects;
	std::vector<std::unique_ptr<Object3d>> iblObjects;
	std::unique_ptr<Object3d> pointLightMarker;
	float pointLightAngle = 0.0f;

};

