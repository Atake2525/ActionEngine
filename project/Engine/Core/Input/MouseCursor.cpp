#include "MouseCursor.h"
#include "Input.h"
#include "RenderObjectFactory.h"
#include "WinApp.h"
#include <algorithm>
#include <cassert>

MouseCursor::MouseCursor() {
}

MouseCursor::~MouseCursor() {
}

void MouseCursor::SetContext(Input& input, WinApp& winApp, SpriteFactory& spriteFactory) {
    m_input = &input;
    m_pWinApp = &winApp;
    m_pSpriteFactory = &spriteFactory;
}

void MouseCursor::Initialize(const std::string& hover, const std::string& press) {
    assert(m_input);
    assert(m_pWinApp);
    assert(m_pSpriteFactory);
    m_cursorTextures[0] = hover;
    m_cursorTextures[1] = press;

    m_cursorSprite = m_pSpriteFactory->Create(m_cursorTextures[0]);
    m_cursorSprite->SetAnchorPoint(ANCHORPOINT_MIDDLETOP);

    m_windowSize = m_pWinApp->GetWindowSize();
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
    if (m_input->PressMouse(MOUSE_LEFT))
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
