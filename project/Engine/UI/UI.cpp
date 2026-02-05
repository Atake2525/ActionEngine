#include "UI.h"
#include "SpriteBase.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "ImGuiManager.h"

using namespace std;

UI::~UI() {
}

void UI::SetSprite(const std::string& filename) {
	TextureManager::GetInstance()->LoadTexture(filename);

	sprite->SetTexture(filename);
}

//明るさ点滅
void UI::SetSpriteAlpha(float alpha)
{
if (sprite) {
		sprite->SetColor({ 1.0f, 1.0f, 1.0f, alpha });  
	}
}

void UI::CreateButton(const Vector2& spritePosition, const Origin& origin, const std::string& filename) {
	input = Input::GetInstance();

	TextureManager::GetInstance()->LoadTexture(filename);

	sprite = make_unique<Sprite>();
	sprite->Initialize(filename);
	sprite->SetPosition(spritePosition);

	switch (origin)
	{
	case Origin::Center:
		sprite->SetAnchorPoint({ 0.5f, 0.5f });
		break;
	case Origin::Top:
		sprite->SetAnchorPoint({ 0.5f, 0.0f });
		break;
	case Origin::Bottom:
		sprite->SetAnchorPoint({ 0.5f, 1.0f });
		break;
	case Origin::LeftTop:
		sprite->SetAnchorPoint({ 0.0f, 0.0f });
		break;
	case Origin::RightTop:
		sprite->SetAnchorPoint({ 1.0f, 0.0f });
		break;
	case Origin::LeftBottom:
		sprite->SetAnchorPoint({ 0.0f, 1.0f });
		break;
	case Origin::RightBottom:
		sprite->SetAnchorPoint({ 1.0f, 1.0f });
		break;
	case Origin::LeftCenter:
		sprite->SetAnchorPoint({ 0.0f, 0.5f });
		break;
	case Origin::RightCenter:
		sprite->SetAnchorPoint({ 1.0f, 0.5f });
		break;
	}
	TriggerOnButton();
}

bool UI::TriggerOnButton() {
	sprite->Update();
	Vector2 spriteOrigin = sprite->GetAnchorPoint();
	Vector2 spriteSize = sprite->GetScale();
	Vector2 spritePos = sprite->GetPosition();
	Vector3 mousePos = input->GetMousePos3();
	AABB spriteAABB = {
		{spritePos.x - (spriteSize.x * spriteOrigin.x), spritePos.y - (spriteSize.y * spriteOrigin.y), 0.0f},
		{spritePos.x + (spriteSize.x * spriteOrigin.x), spritePos.y + (spriteSize.y * spriteOrigin.y), 0.0f},
	};
	if (spriteOrigin.x == 0.0f)
	{
		spriteAABB = {
			{spritePos.x - (spriteSize.x), spritePos.y - (spriteSize.y * spriteOrigin.y), 0.0f},
			{spritePos.x + (spriteSize.x), spritePos.y + (spriteSize.y * spriteOrigin.y), 0.0f},
		};
	}
	if (spriteOrigin.y == 0.0f)
	{
		spriteAABB = {
			{spritePos.x - (spriteSize.x * spriteOrigin.x), spritePos.y - (spriteSize.y), 0.0f},
			{spritePos.x + (spriteSize.x * spriteOrigin.x), spritePos.y + (spriteSize.y), 0.0f},
		};
	}
	AABB windowAABB = WinApp::GetInstance()->GetWindowAABB();
	AABB mousePosAABB;
	// ウィンドウモードに応じて値を少しいじる(ウィンドウの部分を計算に入れる)
	if (WinApp::GetInstance()->GetWindowMode() == WindowMode::Window)
	{
		mousePosAABB = {
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
		};
	}
	else if (WinApp::GetInstance()->GetWindowMode() == WindowMode::FullScreen)
	{
		mousePosAABB = {
			{mousePos.x - windowAABB.min.x, mousePos.y - windowAABB.min.y},
			{mousePos.x - windowAABB.min.x, mousePos.y - windowAABB.min.y},
		};
	}

	if (CollisionAABB(spriteAABB, mousePosAABB) == true && GetAsyncKeyState(VK_LBUTTON) & 0x0001 && GetAsyncKeyState(VK_LBUTTON) == -32768)
	{
		onButton_ = !onButton_;
		return true;
	}

	return false;
}

const bool UI::InCursor() const {
	sprite->Update();
	Vector2 spriteOrigin = sprite->GetAnchorPoint();
	Vector2 spriteSize = sprite->GetScale();
	Vector2 spritePos = sprite->GetPosition();
	Vector3 mousePos = input->GetMousePos3();
	AABB spriteAABB = {
		{spritePos.x - (spriteSize.x * spriteOrigin.x), spritePos.y - (spriteSize.y * spriteOrigin.y), 0.0f},
		{spritePos.x + (spriteSize.x * spriteOrigin.x), spritePos.y + (spriteSize.y * spriteOrigin.y), 0.0f},
	};
	if (spriteOrigin.x == 0.0f)
	{
		spriteAABB = {
			{spritePos.x - (spriteSize.x), spritePos.y - (spriteSize.y * spriteOrigin.y), 0.0f},
			{spritePos.x + (spriteSize.x), spritePos.y + (spriteSize.y * spriteOrigin.y), 0.0f},
		};
	}
	if (spriteOrigin.y == 0.0f)
	{
		spriteAABB = {
			{spritePos.x - (spriteSize.x * spriteOrigin.x), spritePos.y - (spriteSize.y), 0.0f},
			{spritePos.x + (spriteSize.x * spriteOrigin.x), spritePos.y + (spriteSize.y), 0.0f},
		};
	}
	AABB windowAABB = WinApp::GetInstance()->GetWindowAABB();
	AABB mousePosAABB;
	// ウィンドウモードに応じて値を少しいじる(ウィンドウの部分を計算に入れる)
	if (WinApp::GetInstance()->GetWindowMode() == WindowMode::Window)
	{
		mousePosAABB = {
			{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
			{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
		};
	}
	else if (WinApp::GetInstance()->GetWindowMode() == WindowMode::FullScreen)
	{
		mousePosAABB = {
			{mousePos.x - windowAABB.min.x, mousePos.y - windowAABB.min.y},
			{mousePos.x - windowAABB.min.x, mousePos.y - windowAABB.min.y},
		};
	}

	if (CollisionAABB(spriteAABB, mousePosAABB) == true)
	{
		return true;
	}
	/*ImGui::Begin("Button");
	ImGui::DragFloat2("mousePos", &mousePosAABB.min.x, 0.1f);
	ImGui::End();*/

	return false;
}

void UI::Draw() {
	sprite->Draw();
}

void UI::TriggerFunction() {
	if (function)
	{
		function();
	}
}

bool UI::CollisionAABB(const AABB& a, const AABB& b) const {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true;
	}
	return false;
}