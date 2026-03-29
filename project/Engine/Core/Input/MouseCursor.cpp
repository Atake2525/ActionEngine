#include "MouseCursor.h"
#include "Input.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <algorithm>

MouseCursor::MouseCursor() {
}

MouseCursor::~MouseCursor() {
}

void MouseCursor::Initialize(const std::string& hover, const std::string& press) {
    TextureManager::GetInstance()->LoadTexture(hover);
    TextureManager::GetInstance()->LoadTexture(press);
    m_cursorTextures[0] = hover;
    m_cursorTextures[1] = press;
    m_input = Input::GetInstance();

    m_cursorSprite = std::make_unique<Sprite>();
    m_cursorSprite->Initialize(m_cursorTextures[0]);
    m_cursorSprite->SetAnchorPoint(ANCHORPOINT_MIDDLETOP);

    m_windowSize = WinApp::GetInstance()->GetWindowSize();
}

void MouseCursor::SetCursorTextureFilePath(const std::string & hover, const std::string & press) {
    m_cursorTextures[0] = hover;
    m_cursorTextures[1] = press;
}

void MouseCursor::Update() {
    Vector2 pos = m_cursorSprite->GetPosition() + m_input->GetMouseVel2();
    pos = Vector2::Clamp(pos, { 0.0f, 0.0f }, m_windowSize);
    m_cursorPos = pos;
    m_cursorSprite->SetPosition(m_cursorPos);
    if (m_input->PressMouse(0))
    {
        m_cursorSprite->SetTexture(m_cursorTextures[1]);
        Vector2 scale = m_cursorSprite->GetScale();
        m_cursorSprite->SetScale(scale / 3.0f);
    }
    else
    {
        m_cursorSprite->SetTexture(m_cursorTextures[0]);
        Vector2 scale = m_cursorSprite->GetScale();
        m_cursorSprite->SetScale(scale / 3.0f);
    }
    m_cursorSprite->Update();

}

void MouseCursor::Draw() {
    if (m_isShowCursor)
    {
        m_cursorSprite->Draw();
    }
}

void MouseCursor::SetCursorPosition(const Vector2& mousePosition)
{
    m_cursorPos = mousePosition;
    m_cursorSprite->SetPosition(m_cursorPos);
}
