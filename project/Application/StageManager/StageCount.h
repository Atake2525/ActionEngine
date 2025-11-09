#include <memory>
#pragma once


class StageCount
{
private:
	static StageCount* instance;

	StageCount() = default;
	~StageCount() = default;

	StageCount(StageCount&) = delete;
	StageCount& operator=(StageCount&) = delete;

public:
	static StageCount* GetInstance();

	// 終了処理
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	void SetStageCount(int count) { stageCount_ = count; }

	const int& GetStageCount() const { return stageCount_; }

private:
	int stageCount_;

};

