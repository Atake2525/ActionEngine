#pragma once

#include <memory>

class GameScene;

class IGameSceneReadyState
{
public:
	virtual ~IGameSceneReadyState() = default;

	virtual void Enter(GameScene& scene) {}
	virtual void Update(GameScene& scene) = 0;
	virtual void Exit(GameScene& scene) {}
};

std::unique_ptr<IGameSceneReadyState> CreateInitialGameSceneReadyState();
