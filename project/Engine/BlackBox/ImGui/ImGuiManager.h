#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"
#include "algorithm"
#include "WinApp.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

#pragma once


class DirectXBase;

class ImGuiManager
{
private:
    // シングルトンパターンを適用
    static ImGuiManager* instance;

    // コンストラクタ、デストラクタの隠蔽
    ImGuiManager() = default;
    ~ImGuiManager() = default;

    // コピーコンストラクタ、コピー代入演算子の封印
    ImGuiManager(ImGuiManager&) = delete;
    ImGuiManager& operator=(ImGuiManager&) = delete;
public:
    // インスタンスの取得
    static ImGuiManager* GetInstance();

    // 終了処理
    void Finalize();

    // 初期化
    void Initialize(DirectXBase* directxBase);

private:
    DirectXBase* directxBase_;
};

