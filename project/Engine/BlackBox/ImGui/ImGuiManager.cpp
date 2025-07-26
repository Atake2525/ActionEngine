#include "ImGuiManager.h"
#include "DirectXBase.h"
#include "SrvManager.h"
#include "Logger.h"

using namespace Logger;

ImGuiManager* ImGuiManager::instance = nullptr;

ImGuiManager* ImGuiManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new ImGuiManager;
	}
	return instance;
}

void ImGuiManager::Finalize()
{
	delete instance;
	instance = nullptr;
	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Initialize(DirectXBase* directxBase)
{
	directxBase_ = directxBase;

	uint32_t srvIndex = SrvManager::GetInstance()->Allocate();

	if (!SrvManager::GetInstance()->CheckAllocate())
	{
		Log("Cant Allocate");
		return;
	}

	// ImGuiの初期化。詳細はさして重要ではないので開設は省略する
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());
	ImGui_ImplDX12_Init(
		directxBase_->GetDevice().Get(), directxBase_->GetSwapChainDesc().BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, SrvManager::GetInstance()->GetDescriptorHeap().Get(), SrvManager::GetInstance()->GetCPUDescriptorHandle(srvIndex),
		SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex));
	
	// 日本語化
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources/Fonts/BIZ-UDGothicR.ttc", 14.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
}

