#include "Sprite.h"
#include <array>
#include <memory>
#include "MouseCursor.h"
#include "Input.h"
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
    std::array<std::unique_ptr<Sprite>, 3> m_uiSprites;
    std::array<Vector2, 3> m_uiBaseScale;

    std::string m_pressUI = "none";

    Input* m_pInput = nullptr;
    MouseCursor* m_mouseCursor = nullptr;

};

