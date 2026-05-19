#include "Sprite.h"
#include <array>
#include <memory>
#include "MouseCursor.h"
#include "Input.h"
#include "UIElement.h"
#include "UIButton.h"
#pragma once

enum class TitleSceneScreen : int {
    BootScreen = 0,
    TitleScreen = 1
};

class TitleSceneUI {
public:
    TitleSceneUI(MouseCursor* mouseCursor);
    ~TitleSceneUI();

    void Update(const TitleSceneScreen& screen);
    void DrawBootScreen();
    void DrawTitleScreen();

    const std::string GetPressUI() const { return m_pressUI; }

private:
    std::unique_ptr<Sprite> m_pressAnyKeySprite;

    std::string m_pressUI = "none";

    Input* m_pInput = nullptr;
    MouseCursor* m_mouseCursor = nullptr;

    std::unique_ptr<UI::UIElement> m_startButton;
    std::unique_ptr<UI::UIElement> m_exitButton;

};

