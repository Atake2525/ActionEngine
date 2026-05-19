#include "UIElement.h"
#pragma once

namespace UI {

    class Button : public UIElement {
    public:
        Button();
        ~Button() override;
        void Initialize(const std::string textureFilePath, Input& input) override;
        void Update() override;
        void Draw() override;

    private:

        // スプライトの元のサイズを保存するための変数
        Vector2 m_originalSpriteSize = Vector2::Zero;

        

        // ボタンのホバー、クリックなどを更新
        void UpdateInteractionState();
        // リアクションの更新
        void UpdateReactions();

    };

}
