#include "UIElement.h"
#pragma once

namespace UI {

    class Button : public Element {
    public:
        Button();
        ~Button() override;
        void Initialize(const std::string textureFilePath, Input& input) override;
        void Update() override;
        void Draw() override;

    private:
        // ボタンのホバー、クリックなどを更新
        void UpdateInteractionState();
        // リアクションの更新
        void UpdateReactions();

    };

}
