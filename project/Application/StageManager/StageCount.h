#include <memory>
#pragma once

/// <summary>
/// ステージカウント
/// </summary>
namespace ActionEngine {
    namespace Stage {
		class StageCount
		{
		public:
			static StageCount* instance;

			StageCount() = default;
			~StageCount() = default;

			StageCount(StageCount&) = delete;
			StageCount& operator=(StageCount&) = delete;

			static StageCount* GetInstance();

			// 終了処理
			void Finalize();

			/// <summary>
			/// 初期化
			/// </summary>
			void Initialize();

			// ステージカウントのセット
			void SetStageCount(int count) { stageCount_ = count; }

			// ステージカウントの取得
			const int& GetStageCount() const { return stageCount_; }

		private:
			int stageCount_;

		};


    }
}
