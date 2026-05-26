#include "UIButton.h"
#include "Collision.h"

#include "Logger.h"

using namespace Logger;
using namespace UI;

Button::Button()
{}

Button::~Button()
{}

void Button::Initialize(const std::string textureFilePath, Input& input) {
    // UI用のSpriteを作成して初期化する
    m_sprite = std::make_unique<Sprite>();
    m_sprite->Initialize(textureFilePath);
    m_sprite->SetAnchorPoint(ANCHORPOINT_MIDDLE);
    m_originalSpriteSize = m_sprite->GetTextureSize();

    m_pInput = &input;
}

void Button::Update() {
    // アクティベート後の確認フラグをfalseに戻す
    if (m_activated)
    {
        m_activated = false;
    }
    m_controlMode = UpdateControlMode(m_controlMode); // コントロールモードを更新する
    // ボタンが表示されている場合のみ更新する
    switch (m_transitionState)
    {
    case TransitionState::Hidden: // ボタンが非表示の状態の処理

        break;
    case TransitionState::Entering: // ボタンが表示される途中の処理
        if (m_enterReaction) {
            m_enterReaction(*this);
        }
        break;
    case TransitionState::Shown: // ボタンが表示されている状態の処理
        // 使用可能数が0以外であれば処理するようにする
        if (m_usableCount != 0)
        {
            UpdateInteractionState(); // ボタンの状態を更新する
            UpdateMouseCursor(); // マウスカーソルを更新する
            if (m_interactionState != m_interactionStatePre) // ボタンの状態が変化した場合
            {
                UpdateReactions(); // ボタンの状態に応じたリアクションを更新する
            }
        }



        break;
    case TransitionState::Exiting: // ボタンが非表示になる途中の処理
        if (m_exitReaction) {
            m_exitReaction(*this);
        }
        break;
    }

    m_sprite->Update();
}

void Button::Draw() {
    // ボタンが表示されている場合のみ描画する
    if (m_transitionState == TransitionState::Shown || m_transitionState == TransitionState::Entering || m_transitionState == TransitionState::Exiting)
    {
        m_sprite->Draw();
    }
}


void Button::UpdateInteractionState() {
    // 前の状態を保存
    m_interactionStatePre = m_interactionState;

    // マウスカーソルがUIの上にない状態でバインドされた入力があった場合は、状態を変化させない
    if (m_interactionState == InteractionState::Idle && (m_interactBinding.CheckPush(*m_pInput) || m_interactBinding.CheckReturn(*m_pInput)))
    {
        return;
    }

    switch (m_controlMode)
    {
    case ControlMode::Keyboard:
        // キーボード操作時の処理
        if (m_selected)
        {
            m_interactionState = InteractionState::Selected; // 選択状態にする
        }
        else
        {
            m_interactionState = InteractionState::Idle; // 通常状態にする
            m_selectedReactionLocked = false; // 状態のロックを解除する
        }

        break;
    case ControlMode::Mouse:
        // マウス操作時の処理
        // マウスの位置を取得
        Vector2 mousePos = m_pInput->GetWindowMousePos2();
        Vector2 buttonPos = m_sprite->GetPosition();
        Vector2 buttonSize = m_sprite->GetTextureSize();

        // マウスがボタンの上にあるかを確認
        if (CollisionUISprite(buttonPos - buttonSize / 2.0f, buttonSize, mousePos, { 1.0f, 1.0f }))
        {
            m_interactionState = InteractionState::Selected; // 選択状態にする
        }
        else
        {
            m_interactionState = InteractionState::Idle; // 通常状態にする
            m_selectedReactionLocked = false; // 状態のロックを解除する
        }
        break;
    case ControlMode::GamePad:
        // ゲームパッド操作時の処理
        if (m_selected)
        {
            m_interactionState = InteractionState::Selected; // 選択状態にする
        }
        else
        {
            m_interactionState = InteractionState::Idle; // 通常状態にする
            m_selectedReactionLocked = false; // 状態のロックを解除する
        }
        break;
    }

    if (m_interactionState == InteractionState::Selected)
    {
        if (m_interactBinding.CheckPush(*m_pInput)) // 左クリックが押されているかをチェック
        {
            m_interactionState = InteractionState::Pressed; // 押下状態にする
        }
        else if (m_interactBinding.CheckReturn(*m_pInput)) // 押下状態で左クリックが離されたかをチェック
        {
            m_interactionState = InteractionState::Submitted; // 決定状態をリセット
            m_selectedReactionLocked = true; // 状態をロックする
        }
    }
}

void Button::UpdateReactions() {
    switch (m_interactionState) // 現在の状態に応じたリアクションを更新
    {
    case InteractionState::Idle:
        if (m_interactionStatePre == InteractionState::Selected || m_interactionStatePre == InteractionState::Pressed)
        {
            if (m_onSelectedReaction.highlight)
            {
                m_sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // ハイライトをリセット
            }
            if (m_onSelectedReaction.scale)
            {
                m_sprite->SetScale(m_originalSpriteSize); // スケールをリセット
            }
        }
        Log("Idle Reaction\n");
        break;
    case InteractionState::Selected:
        if (!m_selectedReactionLocked)
        {
            if (m_onSelectedReaction.custom)
            {
                m_onSelectedReaction.custom();
            }
            if (m_onSelectedReaction.highlight)
            {
                m_sprite->SetColor(m_onSelectedReaction.highlightColor);
            }
            if (m_onSelectedReaction.scale)
            {
                m_sprite->SetScale({ m_originalSpriteSize * m_onSelectedReaction.scaleAmount });
            }
        }
        Log("Selected Reaction\n");
        break;
    case InteractionState::Pressed:
        if (m_onPressedReaction.custom)
        {
            m_onPressedReaction.custom();
        }
        if (m_onPressedReaction.highlight)
        {
            m_sprite->SetColor(m_onPressedReaction.highlightColor);
        }
        if (m_onPressedReaction.scale)
        {
            m_sprite->SetScale({ m_originalSpriteSize * m_onPressedReaction.scaleAmount });
        }
        Log("Pressed Reaction\n");
        break;
    case InteractionState::Submitted:
        if (m_onSubmittedReaction.custom)
        {
            m_onSubmittedReaction.custom();
        }
        if (m_onSubmittedReaction.highlight)
        {
            m_sprite->SetColor(m_onSubmittedReaction.highlightColor);
        }
        if (m_onSubmittedReaction.scale)
        {
            m_sprite->SetScale({ m_originalSpriteSize * m_onSubmittedReaction.scaleAmount });
        }
        if (m_activeReaction)
        {
            m_activeReaction(); // ボタンが押されたときのリアクションを呼び出す
        }
        m_activated = true;
        if (m_usableCount < 0)
        {
            m_usableCount--; // 使用回数を減らす
        }
        Log("Submitted Reaction\n");
        Log("Active Reaction\n");
        break;
    }

}
