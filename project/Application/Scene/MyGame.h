#include "FrameWork.h"
#include "BaseScene.h"
#include "D3DResourceLeakChecker.h"
#include "WinApp.h"
#include "DirectXBase.h"
#include "Input.h"
#include "Render2DBase.h"
#include "Object3dBase.h"
#include "ModelBase.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Light.h"
#include "Audio.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "SkyBox.h"
#include "JsonLoader.h"
#include "CollisionManager.h"
#include "FadeManager.h"
#include "GameTime.h"
#include "StageCount.h"
#include "DebugLineBase.h"

#include "algorithm"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//#include "GameScene.h"

#pragma once

// ゲームループ
class MyGame : public FrameWork {
public:
	// 初期化
	void Initialize() override;

	// 終了処理
	void Finalize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// ループ終了
	bool LoopOut() override { return finished; }
	

	// ↑までシーンの作成に必須

private:
	D3DResourceLeakChecker d3dResourceLeakChecker;

	// メンバ変数宣言

#pragma region 基盤システム

	bool finished = false;

	//BaseScene* gameScene = nullptr;
	// 
	//SceneManager* sceneManager_ = nullptr;

};