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

	// I—¹ˆ—
	void Finalize();

	/// <summary>
	/// ‰Šú‰»
	/// </summary>
	void Initialize();

	void SetStageCount(int count) { stageCount_ = count; }

	const int& GetStageCount() const { return stageCount_; }

private:
	int stageCount_;

};

