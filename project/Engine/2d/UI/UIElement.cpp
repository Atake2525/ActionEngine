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
    if (m_staticControlMode || m_pInput == nullptr)
    {
        return mode;
    }

    if (m_pInput->PressAnyKey())
    {
        return ControlMode::Keyboard;
    }

    if (m_pInput->IsConnectedController())
    {
        if (m_pInput->PressAnyButton() ||
            m_pInput->PushDPad(DPad::Up) ||
            m_pInput->PushDPad(DPad::Down) ||
            m_pInput->PushDPad(DPad::Left) ||
            m_pInput->PushDPad(DPad::Right) ||
            m_pInput->PushDPad(DPad::UpRight) ||
            m_pInput->PushDPad(DPad::UpLeft) ||
            m_pInput->PushDPad(DPad::DownRight) ||
            m_pInput->PushDPad(DPad::DownLeft))
        {
            return ControlMode::GamePad;
        }
    }

    const Vector2 mouseVelocity = m_pInput->GetMouseVel2();
    if (mouseVelocity.x != 0.0f ||
        mouseVelocity.y != 0.0f ||
        m_pInput->PressMouse(MOUSE_LEFT) ||
        m_pInput->PressMouse(MOUSE_RIGHT) ||
        m_pInput->PressMouse(MOUSE_MIDDLE))
    {
        return ControlMode::Mouse;
    }

    return mode;
}

void UIElement::UpdateMouseCursor() {
    if (m_pInput == nullptr || m_controlMode != ControlMode::Mouse)
    {
        return;
    }

    if (m_interactionState == InteractionState::Selected)
    {
        SetCursor(LoadCursor(nullptr, IDC_HAND));

    }
}