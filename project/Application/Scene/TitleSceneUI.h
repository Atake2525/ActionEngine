#include "Sprite.h"
#include <array>
#include <memory>
#include "Input.h"

#include "UISelectionGroup.h"

#pragma once

enum class TitleSceneScreen : int {
    BootScreen = 0,
    TitleScreen = 1
};

class TitleSceneUI {
public:
    TitleSceneUI();
    ~TitleSceneUI();

    void Update(const TitleSceneScreen& screen);
    void DrawBootScreen();
    void DrawTitleScreen();

    const std::string GetPressUI() const { return m_pressUI; }

private:
    std::unique_ptr<Sprite> m_pressAnyKeySprite;

    std::string m_pressUI = "none";

    Input* m_pInput = nullptr;

    std::unique_ptr<UI::SelectionGroup> m_selectionGroup;

};

