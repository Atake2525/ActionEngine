#include "UIElement.h"

#pragma once

class UISlider : public UI::UIElement {
public:
    UISlider();
    ~UISlider();
    void Initialize(const std::string textureFilePath, Input& input) override;
    void Update() override;
    void Draw() override;
};
