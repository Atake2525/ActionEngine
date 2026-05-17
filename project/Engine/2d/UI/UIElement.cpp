#include "UIElement.h"

void UIElement::SetPosition(const Vector2& pos) {
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return; // Spriteが存在しない場合は何もしない
    }
    m_sprite->SetPosition(pos);
    m_sprite->Update(); // 位置を変更した後にSpriteを更新する
}

const Vector2& UIElement::GetPosition() const
{
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return Vector2::Zero; // Spriteが存在しない場合はゼロベクトルを返す
    }
    return m_sprite->GetPosition();
}

void UIElement::SetScale(const Vector2& scale) {
    // まずはSpriteが存在するか確認する
    if (!m_sprite) {
        return; // Spriteが存在しない場合は何もしない
    }
    m_sprite->SetScale(scale);
    m_sprite->Update(); // 拡縮を変更した後にSpriteを更新する
}

const Vector2& UIElement::GetScale() const {
    // まずはSpriteが存在するか確認する
    if (!m_sprite)
    {
        return Vector2::Zero; // Spriteが存在しない場合はゼロベクトルを返す
    }
    return m_sprite->GetScale(); // 拡縮を取得する
}
