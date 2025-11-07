#pragma once

#include "Sprite.h"
#include "UI.h"
#include "Input.h"
#include "JsonLoader.h"
#include "Object3d.h"
#include <memory>

class Goal
{
public:

	~Goal();

	void Initalize();

	void Update();

	void Draw();

private:

	Input* input = nullptr;

	std::vector<JsonData> jsonDatas;

	std::vector<std::unique_ptr<Object3d>> goalObjects;

};

