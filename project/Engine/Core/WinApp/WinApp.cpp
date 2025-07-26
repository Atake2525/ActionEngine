#include "WinApp.h"
#include "externels/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

WinApp* WinApp::instance = nullptr;

WinApp* WinApp::GetInstance() {
	if (instance == nullptr)
	{
		instance = new WinApp;
	}
	return instance;
}

// ウィンドウプロージャ
LRESULT CALLBACK WinApp::windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが放棄された
	case WM_DESTROY:
		// osに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準メッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize(const int32_t& width, const uint32_t& height, WindowMode windowMode, const wchar_t* windowname) {

	kClientWidth = width;
	kClientHeight = height;
	this->windowMode = windowMode;


	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);

	// ウィンドウプロージャ
	wc.lpfnWndProc = windowProc;
	// ウィンドウのクラス名
	wc.lpszClassName = L"CGWindowClass";
	// インスタントハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);


	// ウィンドウモード、サイズの変更を初期化で行えるようにする

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = {0, 0, kClientWidth, kClientHeight};

	if (windowMode == WindowMode::FullScreen) {
		// クライアント領域をもとに実際のサイズにwrcを変更してもらう
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// WS_OVERLAPPEDWINDOW ウィンドウ
		// WS_POPUP フルスクリーン

		// ウィンドウの生成
		hwnd = CreateWindow(wc.lpszClassName, windowname, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

		// ウィンドウを表示する
		ShowWindow(hwnd, SW_MAXIMIZE);
	} else {
		// クライアント領域をもとに実際のサイズにwrcを変更してもらう
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// WS_OVERLAPPEDWINDOW ウィンドウ
		// WS_POPUP フルスクリーン

		// ウィンドウの生成
		hwnd = CreateWindow(wc.lpszClassName, windowname, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

		// ウィンドウを表示する
		ShowWindow(hwnd, SW_SHOW);
		
	}

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
}

const AABB WinApp::GetWindowAABB() const {
	RECT windowRect;
	RECT rt;
	if (GetWindowRect(hwnd, &windowRect)) {
		rt = windowRect;
	}
	if (rt.right < 0 || rt.bottom < 0)
	{
		AABB none = { {0.0f, 0.0f}, {0.0f, 0.0f} };
		return none;
	}
	AABB result = {
		{static_cast<float>(rt.left), static_cast<float>(rt.top)},
		{static_cast<float>(rt.right), static_cast<float>(rt.bottom)}
	};

	return result;
}

void WinApp::Update() {
	RECT wrc = {0, 0, 1920, 1080};

		// クライアント領域をもとに実際のサイズにwrcを変更してもらう
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// WS_OVERLAPPEDWINDOW ウィンドウ
		// WS_POPUP フルスクリーン

		// ウィンドウの生成
		hwnd = CreateWindow(wc.lpszClassName, L"Base Engine", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

		// ウィンドウを表示する
		ShowWindow(hwnd, SW_MAXIMIZE);
}

// 終了
void WinApp::Finalize() { 
	CloseWindow(hwnd);
	CoUninitialize();

	delete instance;
	instance = nullptr;
}

// メッセージの処理
bool WinApp::ProcessMessage() {
	MSG msg{};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
	{
		 return true;
	}

	return false; 
}