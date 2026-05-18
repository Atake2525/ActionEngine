#include "UIElement.h"
#pragma once

namespace UI {
    struct ButtonReaction
    {
        bool highlight = false; // ハイライトリアクション
        Vector4 highlightColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // ハイライトカラー
        bool scale = false; // スケールリアクション
        Vector2 scaleAmount = { 1.0f, 1.0f }; // スケール倍率
        std::function<void()> custom; // カスタムリアクション
    };

    enum class ButtonState {
        Normal = 0, // 通常状態
        Hovered,    // ホバー状態
        Clicked,     // クリック状態
        Released,    // 離された状態
    };

    class Button : public UIElement {
    public:
        Button();
        ~Button();
        void Initialize(const std::string& textureFilePath, Input& input) override;
        void Update() override;
        void Draw() override;

        void SetActiveReaction(const std::function<void()>& reaction) { m_activeReaction = reaction; }

        void SetOnHoverReaction(const ButtonReaction& reaction) { m_onHoverReaction = reaction; }

    private:
        std::function<void()> m_activeReaction; // ボタンが押されたときに呼び出される関数

        // スプライトの元のサイズを保存するための変数
        Vector2 m_originalSpriteSize = Vector2::Zero;

        ButtonReaction m_onHoverReaction; // ホバー状態のリアクション
        ButtonReaction m_onClickReaction; // クリック状態のリアクション
        ButtonReaction m_onReleaseReaction; // 離された状態のリアクション

        ButtonState m_buttonStatePre = ButtonState::Normal; // ボタンの前の状態
        ButtonState m_buttonState = ButtonState::Normal; // ボタンの現在の状態

        // ボタンのホバー、クリックなどを更新
        void UpdateButtonState();
        // リアクションの更新
        void UpdateReactions();

    };

}
