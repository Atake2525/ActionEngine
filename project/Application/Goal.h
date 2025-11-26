#pragma once

#include "Sprite.h"
#include "UI.h"
#include "Input.h"
#include "JsonLoader.h"
#include "Object3d.h"
#include <memory>
#include "AABB.h"

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
	void Initalize();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="aabb">処理の対象となるAABB</param>
	void Update(AABB aabb);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 対象が触れてゴール判定になったかどうか
	/// </summary>
	/// <returns></returns>
	const bool& IsGoal() const { return isGoal_; }

private:

	Input* input = nullptr;

	std::vector<JsonData> jsonDatas;

	std::vector<std::unique_ptr<Object3d>> goalObjects;

	bool isGoal_ = false;

};

