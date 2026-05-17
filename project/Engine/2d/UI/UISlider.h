#include "UIElement.h"

#pragma once

class UISlider : public UIElement {
public:
    UISlider();
    ~UISlider();
    void Initialize(const std::string& textureFilePath, Input& input) override;
    void Update() override;
    void Draw() override;
};
