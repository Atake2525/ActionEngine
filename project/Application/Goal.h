#pragma once

#include "Sprite.h"
#include "UI.h"
#include "Input.h"
#include "JsonLoader.h"
#include "Object3d.h"
#include <memory>
#include "AABB.h"
#include "Result.h"

class Player;
class MouseCursor;

/// <summary>
/// ゴール
/// </summary>
class Goal
{
public:
	// デストラクタ
	~Goal();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string jsonName, Player* player, MouseCursor* mouseCursor);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="aabb">処理の対象となるAABB</param>
	const bool& ChceckIsGoal();

	/// <summary>
	/// ゴールオブジェクト描画
	/// </summary>
	void DrawGoalObject();

	void DrawResult();

#ifndef NDEBUG
	void SetGoal() { m_isGoal = true; }
#endif // !NDEBUG


private:
	// 全体で使うメンバ変数
	Input* m_input = nullptr;
	Player* m_player = nullptr;
	std::vector<JsonData> m_jsonDatas;
	bool m_isGoal = false;

	std::unique_ptr<Result> m_result;

	// オブジェクト(ゴール)関係
	std::vector<std::unique_ptr<Object3d>> m_goalObjects;

	// スプライト(リザルト)関係
	std::vector<std::unique_ptr<Sprite>> m_resultSprites;

	// 演出関係
	bool m_goalEffectStart = false;
	float m_goalEffectTimer = 0.0f;
	float m_goalEffectTime = 1.0f;
};

