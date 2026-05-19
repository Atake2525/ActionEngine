#include "UIElement.h"

using namespace UI;

void UIElement::SetPosition(const Vector2 pos) {
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return; // Spriteが存在しない場合は何もしない
    }
    m_sprite->SetPosition(pos);
    m_sprite->Update(); // 位置を変更した後にSpriteを更新する
}

const Vector2 UIElement::GetPosition() const
{
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return Vector2::Zero; // Spriteが存在しない場合はゼロベクトルを返す
    }
    return m_sprite->GetPosition();
}

void UIElement::SetScale(const Vector2 scale) {
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return; // Spriteが存在しない場合は何もしない
    }
    m_sprite->SetScale(scale);
    m_sprite->Update(); // 拡縮を変更した後にSpriteを更新する
}

const Vector2 UIElement::GetScale() const {
    // まずはSpriteが存在するか確認する
    if (!m_sprite)
    {
        return Vector2::Zero; // Spriteが存在しない場合はゼロベクトルを返す
    }
    return m_sprite->GetScale(); // 拡縮を取得する
}

void UI::UIElement::SetRotation(float rotation) {
    if (!m_sprite)
    {
        return;
    }
    m_sprite->SetRotatioin(rotation);
    m_sprite->Update(); // 回転を変更した後にSpriteを更新する
}

const float UI::UIElement::GetRotation() const {
    if (!m_sprite)
    {
        return 0.0f;// Spriteが存在しない場合はゼロを返す
    }
    return m_sprite->GetRotation(); // 回転を取得する
}

const ControlMode UI::UIElement::UpdateControlMode(ControlMode mode) const {
    if (!m_staticControlMode)
    {
        if (m_pInput->PressAnyKey() && mode != ControlMode::Keyboard)
        {
            return ControlMode::Keyboard;
        }
        if (m_interactionState == InteractionState::Selected && mode != ControlMode::Mouse)
        {
            return ControlMode::Mouse;
        }
    }
    return mode;
}
