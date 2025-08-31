#pragma once

#include "Sprite.h"
#include "UI.h"
#include "Input.h"

class Goal
{
public:

	~Goal();

	void Initalize();

	void Update();

	void Draw();

private:
	Sprite* clearBackScreenSprite_ = nullptr;

	Sprite* clearTextSprite_ = nullptr;

	UI* titleUI_ = nullptr;

	Input* input = nullptr;

};

