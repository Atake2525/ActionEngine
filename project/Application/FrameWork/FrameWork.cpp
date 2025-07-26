#include "FrameWork.h"

void FrameWork::Initialize() {
	SceneManager::GetInstance();
}

void FrameWork::Update() {
	//sceneManager_->Update();
}

void FrameWork::Finalize() {
	//delete sceneManager_;
	SceneManager::GetInstance()->Finalize();
}

void FrameWork::Run() {
	// 初期化
	Initialize();

	while (true) {
		// 更新
		Update();
		// ループ脱出
		if (RoopOut()) {
			break;
		}
		// 描画
		Draw();
	}
	// 終了処理
	Finalize();
}