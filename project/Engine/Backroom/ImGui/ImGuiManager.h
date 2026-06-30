#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "algorithm"
#include "WinApp.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

#pragma once

class DirectXBase;
class SrvManager;
class WinApp;

// ImGui初期化
class ImGuiManager
{
public:
    ImGuiManager();
    ~ImGuiManager();
    // 初期化
    void Initialize(DirectXBase& directXBase, SrvManager& srvManager, WinApp& winApp);

    void Update();

    void BeginDockSpace();

private:
    DirectXBase* m_pDirectXBase;
    SrvManager* m_pSrvManager;
    WinApp* m_pWinApp;
};

