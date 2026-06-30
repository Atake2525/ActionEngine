#include "FrameWork.h"

void FrameWork::Initialize() {
	m_runtime = std::make_unique<Runtime>();
}

void FrameWork::Update() {
	m_runtime->Update();
}

void FrameWork::Finalize() {
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
		m_runtime->Draw();
	}
	// 終了処理
	Finalize();
}