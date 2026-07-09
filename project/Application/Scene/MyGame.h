#include "FrameWork.h"
#include "D3DResourceLeakChecker.h"

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

	// ↑までシーンの作成に必須

private:
	D3DResourceLeakChecker d3dResourceLeakChecker;

};