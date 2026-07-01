#include "FrameWork.h"

void FrameWork::Initialize() {
	m_pRuntime = std::make_unique<Runtime>();
	m_pRuntime->Initialize();
}

void FrameWork::Update() {
	m_pRuntime->Update();
	loopOut_ = m_pRuntime->EndRequest();
}

void FrameWork::Finalize() {
	m_pRuntime.reset();
}

void FrameWork::Run() {
	// 初期化
	Initialize();

	while (true) {
		// 更新
		Update();
		// ループ脱出
		if (LoopOut()) {
			break;
		}
		// 描画
		m_pRuntime->Draw();
	}
	// 終了処理
	Finalize();
}
