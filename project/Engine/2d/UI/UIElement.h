#include <memory>
#include <functional>
#include "Sprite.h"
#include "Input.h"

#pragma once

namespace UI {
    // UIの状態を管理するクラス
    enum class TransitionState {
        Hidden = 0, // 非表示
        Entering,   // 表示中
        Shown,      // 表示
        Exiting     // 非表示にする途中
    };

    struct InteractionReaction // リアクションを管理する構造体
    {
        bool highlight = false; // ハイライトリアクション
        Vector4 highlightColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // ハイライトカラー
        bool scale = false; // スケールリアクション
        Vector2 scaleAmount = { 1.0f, 1.0f }; // スケール倍率
        std::function<void()> custom; // カスタムリアクション
    };

    enum class InteractionState { // UIの状態を管理するクラス
        Idle = 0, // 通常状態
        Selected,    // 選択状態
        Pressed,     // 押下状態
        Submitted,    // 決定状態
    };

    enum class ControlMode {
        Keyboard,   // キーボードで操作
        Mouse,      // マウスで操作
        GamePad,    // ゲームパッドで操作
    };

    struct InputTrigger { // 入力トリガーを管理する構造体
        BYTE key = 0;
        int mouseButton = 0;
        Controller controller = Controller::None;
        DPad dpad = DPad::None;
    };

    struct InputBinding { // 入力バインディングを管理する構造体
        std::vector<InputTrigger> triggers;

        bool Check(Input& input) const {
            for (const auto& trigger : triggers) {
                if (trigger.key != 0 && input.TriggerKey(trigger.key)) {
                    return true;
                }
                if (trigger.mouseButton != 0 && input.TriggerMouse(trigger.mouseButton)) {
                    return true;
                }
                if (trigger.controller != Controller::None && input.TriggerButton(trigger.controller)) {
                    return true;
                }
                if (trigger.dpad != DPad::None && input.TriggerXButton(trigger.dpad)) {
                    return true;
                }
            }
            return false;
        }
    };

    class UIElement
    {
    public:
        ~UIElement() = default;

        virtual void Initialize(const std::string textureFilePath, Input& input) = 0;
        virtual void Update() = 0;
        virtual void Draw() = 0;

        // リアクションの設定関数群
        void SetEnterReaction(const std::function<void()>& reaction) { m_enterReaction = reaction; }
        void SetExitReaction(const std::function<void()>& reaction) { m_exitReaction = reaction; }
        void SetTransitionState(TransitionState state) { m_transitionState = state; }

        void SetActiveReaction(const std::function<void()>& reaction) { m_activeReaction = reaction; }
        void SetOnSelectedReaction(const InteractionReaction& reaction) { m_onSelectedReaction = reaction; }
        void SetOnPressedReaction(const InteractionReaction& reaction) { m_onPressedReaction = reaction; }
        void SetOnSubmittedReaction(const InteractionReaction& reaction) { m_onSubmittedReaction = reaction; }

        // 状態変更関数群
        void ShowThisFrame() { m_transitionState = TransitionState::Shown; }
        void HideThisFrame() { m_transitionState = TransitionState::Hidden; }
        void Show() { m_transitionState = TransitionState::Entering; }
        void Hide() { m_transitionState = TransitionState::Exiting; }

        // コントロールモードの設定関数
        void SetControlMode(ControlMode mode) { m_controlMode = mode; }
        const ControlMode& GetControlMode() const { return m_controlMode; }
        void StaticControlMode(ControlMode mode) { m_controlMode = mode; m_staticControlMode = true; } 
        void UnstaticControlMode() { m_staticControlMode = false; }

        // 操作系関数群
        void SetPosition(const Vector2 pos);
        const Vector2 GetPosition() const;
        void SetScale(const Vector2 scale);
        const Vector2 GetScale() const;
        void SetRotation(float rotation);
        const float GetRotation() const;

    protected:

        const ControlMode UpdateControlMode(ControlMode mode) const; // コントロールモードを更新する関数

    protected:
        Input* m_pInput = nullptr;
        TransitionState m_transitionState = TransitionState::Hidden;
        ControlMode m_controlMode = ControlMode::Mouse;
        bool m_staticControlMode = false; // コントロールモードを固定するかどうか
        bool m_selected = false; // 選択されているかどうか(キーボード、ゲームパッド操作のため)
        InteractionState m_interactionStatePre = InteractionState::Idle; // ボタンの前の状態
        InteractionState m_interactionState = InteractionState::Idle; // ボタンの現在の状態

        std::function<void()> m_activeReaction; // ボタンが押されたときに呼び出される関数
        InteractionReaction m_onSelectedReaction; // 選択状態のリアクション
        InteractionReaction m_onPressedReaction; // 押下状態のリアクション
        InteractionReaction m_onSubmittedReaction; // 離された状態のリアクション

        std::unique_ptr<Sprite> m_sprite;
        std::function<void()> m_enterReaction;
        std::function<void()> m_exitReaction;

        BYTE m_interactionKey = 0; // キーボード操作のためのキー番号
        int m_interactionMouseButton = 0; // マウス操作のためのボタン番号
        Controller m_interactionControllerButton = Controller::None; // ゲームパッド操作のためのボタン番号
    };
};