#include "UI.h"
#include "SpriteBase.h"
#include "TextureManager.h"
#include "WinApp.h"

#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

UI::~UI() {
	delete sprite;
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

	sprite = new Sprite();
	sprite->Initialize(filename);
	sprite->SetPosition(spritePosition);

	switch (origin)
	{
	case Origin::Center:
		sprite->SetTextureLeftTop({ 0.5f, 0.5f });
		break;
	case Origin::Top:
		sprite->SetTextureLeftTop({ 0.5f, 0.0f });
		break;
	case Origin::Bottom:
		sprite->SetTextureLeftTop({ 0.5f, 1.0f });
		break;
	case Origin::LeftTop:
		sprite->SetTextureLeftTop({ 0.0f, 0.0f });
		break;
	case Origin::RightTop:
		sprite->SetTextureLeftTop({ 1.0f, 0.0f });
		break;
	case Origin::LeftBottom:
		sprite->SetTextureLeftTop({ 0.0f, 1.0f });
		break;
	case Origin::RightBottom:
		sprite->SetTextureLeftTop({ 1.0f, 1.0f });
		break;
	case Origin::LeftCenter:
		sprite->SetTextureLeftTop({ 0.0f, 0.5f });
		break;
	case Origin::RightCenter:
		sprite->SetTextureLeftTop({ 1.0f, 0.5f });
		break;
	}
}

bool UI::OnButton() {
	sprite->Update();
	Vector2 spriteOrigin = sprite->GetTextureLeftTop();
	Vector2 spriteSize = sprite->GetScale();
	Vector2 spritePos = sprite->GetPosition();
	Vector3 mousePos = input->GetMousePos3();
	AABB spriteAABB = {
		{spritePos.x, spritePos.y, 0.0f},
		{spritePos.x + spriteSize.x, spritePos.y + spriteSize.y, 0.0f},
	};
	AABB windowAABB = WinApp::GetInstance()->GetWindowAABB();
	AABB mousePosAABB = {
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
	};

	if (CollisionAABB(spriteAABB, mousePosAABB) == true && GetAsyncKeyState(VK_LBUTTON) == -32768)
	{
		return true;
	}
	/*ImGui::Begin("Button");
	ImGui::DragFloat2("mousePos", &mousePosAABB.min.x, 0.1f);
	ImGui::End();*/

	return false;
}

const bool UI::InCursor() const {
	sprite->Update();
	Vector2 spriteOrigin = sprite->GetTextureLeftTop();
	Vector2 spriteSize = sprite->GetScale();
	Vector2 spritePos = sprite->GetPosition();
	Vector3 mousePos = input->GetMousePos3();
	AABB spriteAABB = {
		{spritePos.x, spritePos.y, 0.0f},
		{spritePos.x + spriteSize.x, spritePos.y + spriteSize.y, 0.0f},
	};
	AABB windowAABB = WinApp::GetInstance()->GetWindowAABB();
	AABB mousePosAABB = {
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
		{mousePos.x - windowAABB.min.x - 8.0f, mousePos.y - windowAABB.min.y - 30.0f},
	};

	if (CollisionAABB(spriteAABB, mousePosAABB) == true)
	{
		return true;
	}
	//ImGui::Begin("Button");
	//ImGui::DragFloat2("mousePos", &mousePosAABB.min.x, 0.1f);
	//ImGui::End();

	return false;
}

void UI::Draw() {
	sprite->Draw();
}

bool UI::CollisionAABB(const AABB& a, const AABB& b) const {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true;
	}
	return false;
}