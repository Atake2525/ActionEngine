#include "ImGuiManager.h"
#include "DirectXBase.h"
#include "SrvManager.h"
#include "Logger.h"

using namespace Logger;

ImGuiManager::ImGuiManager() {}

ImGuiManager::~ImGuiManager() {
	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Initialize(DirectXBase& directXBase, SrvManager& srvManager, WinApp& winApp) {
	m_pDirectXBase = &directXBase;
	m_pSrvManager = &srvManager;
	m_pWinApp = &winApp;

	uint32_t srvIndex = m_pSrvManager->Allocate();

	if (!m_pSrvManager->CheckAllocate())
	{
		Log("Cant Allocate");
		return;
	}

	// ImGuiの初期化。詳細はさして重要ではないので開設は省略する
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = false;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMenuButtonPosition = ImGuiDir_Left;

	ImGui_ImplWin32_Init(m_pWinApp->GetHwnd());
	ImGui_ImplDX12_Init(
		m_pDirectXBase->GetDevice().Get(), m_pDirectXBase->GetSwapChainDesc().BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_pSrvManager->GetDescriptorHeap().Get(), m_pSrvManager->GetCPUDescriptorHandle(srvIndex),
		m_pSrvManager->GetGPUDescriptorHandle(srvIndex));
	
	// 日本語化
	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources/Fonts/BIZ-UDGothicR.ttc", 14.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
}


void ImGuiManager::Update() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	BeginDockSpace();
}

void ImGuiManager::BeginDockSpace()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) == 0)
	{
		return;
	}

	const ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspaceFlags);
}

