#include "runtime.h"
#include <memory>

#pragma once

class FrameWork {
public:
	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 毎フレーム更新
	virtual void Update();

	// 描画
	virtual void Draw() = 0;

	// 実行
	void Run();

	// 終了チェック
	virtual bool LoopOut() { return loopOut_; }

	virtual ~FrameWork() = default;

private:
	bool loopOut_ = false;
protected:
	std::unique_ptr<Runtime> m_pRuntime;

};
