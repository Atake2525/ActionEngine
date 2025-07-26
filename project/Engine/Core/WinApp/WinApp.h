#include <Windows.h>
#include <cstdint>
#include "AABB.h"
#include <string>

#pragma comment(lib, "winmm.lib")
#pragma once

enum class WindowMode {
	Window,
	FullScreen,
};

class WinApp {
private:
	static WinApp* instance;

	WinApp() = default;
	~WinApp() = default;

	WinApp(WinApp&) = delete;
	WinApp& operator=(WinApp&) = delete;

public:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:

	static WinApp* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const int32_t& width = 1280, const uint32_t& height = 720, WindowMode windowMode = WindowMode::Window, const wchar_t* windowname = L"Base Engine");

	void Update();

	// 終了
	void Finalize();

	// メッセージの処理
	bool ProcessMessage();

	const AABB GetWindowAABB() const;

	// クライアント領域サイズ
	int32_t kClientWidth = 1920;
	int32_t kClientHeight = 1080;

	const int32_t& GetkClientWidth() const { return kClientWidth; }
	const int32_t& GetkClientHeight() const { return kClientHeight; }

	WindowMode windowMode = WindowMode::FullScreen;

	// getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }

private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc{};
};
