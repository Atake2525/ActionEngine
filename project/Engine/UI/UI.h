#include "Vector2.h"
#include "Sprite.h"
#include "Input.h"
#include <string>
#include "AABB.h"

#pragma once

// 当たり判定のタイプ
//enum ButtonType {
//	Box,
//	Circle,
//	Triangle,
//};

// 原点
enum Origin {
	Center,
	Top,
	Bottom,
	LeftTop,
	RightTop,
	LeftBottom,
	RightBottom,
	LeftCenter,
	RightCenter,
};

class UI {
public:
	~UI();
	/// <summary>
	/// ボタンの追加
	/// </summary>
	/// <param name="spritePosition">ボタンの位置</param>
	/// <param name="origin">ボタンの原点</param>
	/// <param name="filename">スプライトのファイル名</param>
	void CreateButton(const Vector2& spritePosition, const Origin& origin, const std::string& filename/*, const ButtonType& buttonType*/);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ボタンが押されたかどうか
	/// </summary>
	/// <returns>押下のbool</returns>
	bool OnButton();

	/// <summary>
	/// ボタンにカーソルが合わせられているかどうか
	/// </summary>
	/// <returns>衝突判定のbool</returns>
	const bool InCursor() const;

	void SetTransform(const Transform& transform) { sprite->SetTransform(transform); }

	const Transform& GetTransform() const { return sprite->GetTransform(); }

	void SetSprite(const std::string& filename);

	// 追加
	void SetSpriteAlpha(float alpha);

private:
	bool CollisionAABB(const AABB& a, const AABB& b) const;

	Sprite* sprite = nullptr;
	Input* input = nullptr;
};

