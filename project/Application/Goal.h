#pragma once

#include "Sprite.h"
#include "UI.h"
#include "Input.h"
#include "JsonLoader.h"
#include "Object3d.h"
#include <memory>
#include "AABB.h"

class Goal
{
public:

	~Goal();

	void Initalize();

	void Update(AABB aabb);

	void Draw();

	const bool& IsGoal() const { return isGoal_; }

private:

	Input* input = nullptr;

	std::vector<JsonData> jsonDatas;

	std::vector<std::unique_ptr<Object3d>> goalObjects;

	bool isGoal_ = false;

};

