#include "StageCount.h"
#include "Logger.h"

using namespace std;
using namespace Logger;
using namespace ActionEngine::Stage;

StageCount* StageCount::instance = nullptr;

StageCount* StageCount::GetInstance() {
	if (!instance)
	{
		instance = new StageCount;
	}
	return instance;
}

void StageCount::Initialize() {
	stageCount_ = 0;
}

void StageCount::Finalize() {
	delete instance;
	instance = nullptr;
}
