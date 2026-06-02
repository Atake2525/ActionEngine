#include "GameSceneReadyState.h"
#include "GameScene.h"

namespace {
	class ExpandCameraRangeReadyState final : public IGameSceneReadyState
	{
	public:
		void Update(GameScene& scene) override;
	};

	class ExpandScanEffectReadyState final : public IGameSceneReadyState
	{
	public:
		void Update(GameScene& scene) override;
	};

	class CompletedReadyState final : public IGameSceneReadyState
	{
	public:
		void Enter(GameScene& scene) override;
		void Update(GameScene& scene) override;
	};

	void ExpandCameraRangeReadyState::Update(GameScene& scene)
	{
		const float progress = scene.AdvanceReadyPresentationTimer();
		scene.ExpandReadyCameraRange(progress);

		if (progress >= 1.0f)
		{
			scene.ChangeReadyState(std::make_unique<ExpandScanEffectReadyState>());
		}
	}

	void ExpandScanEffectReadyState::Update(GameScene& scene)
	{
		const float progress = scene.AdvanceReadyPresentationTimer();
		scene.ExpandReadyScanEffect(progress);

		if (progress >= 1.0f)
		{
			scene.ChangeReadyState(std::make_unique<CompletedReadyState>());
		}
	}

	void CompletedReadyState::Enter(GameScene& scene)
	{
		scene.CompleteReadyPresentation();
	}

	void CompletedReadyState::Update(GameScene& scene)
	{
	}
}

std::unique_ptr<IGameSceneReadyState> CreateInitialGameSceneReadyState()
{
	return std::make_unique<ExpandCameraRangeReadyState>();
}
