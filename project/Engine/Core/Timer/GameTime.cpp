#include "GameTime.h"
#include <chrono>

GameTime* GameTime::instance = nullptr;

GameTime* GameTime::GetInstance() {
	if (instance == nullptr) {
		instance = new GameTime;
	}
	return instance;
}

void GameTime::Finalize() {
	delete instance;
	instance = nullptr;
}

void GameTime::Initialize() {
	deltaTime = 0.0f;
}

void GameTime::Update()
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = currentTime - lastTime;
	lastTime = currentTime;
	deltaTime = delta.count();
}