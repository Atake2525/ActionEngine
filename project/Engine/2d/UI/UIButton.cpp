#include "UIButton.h"
#include "Collision.h"

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
    m_controlMode = UpdateControlMode(m_controlMode); // コントロールモードを更新する
    // ボタンが表示されている場合のみ更新する
    switch (m_transitionState)
    {
    case TransitionState::Hidden: // ボタンが非表示の状態の処理

        break;
    case TransitionState::Entering: // ボタンが表示される途中の処理
        m_enterReaction();
        break;
    case TransitionState::Shown: // ボタンが表示されている状態の処理

        UpdateInteractionState(); // ボタンの状態を更新する
        if (m_interactionState != m_interactionStatePre) // ボタンの状態が変化した場合
        {
            UpdateReactions(); // ボタンの状態に応じたリアクションを更新する
        }


        break;
    case TransitionState::Exiting: // ボタンが非表示になる途中の処理
        m_exitReaction();
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

    switch (m_controlMode)
    {
        case ControlMode::Keyboard:
            // キーボード操作時の処理

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
                if (m_pInput->TriggerMouse(0)) // 左クリックが押されているかをチェック
                {
                    m_interactionState = InteractionState::Pressed; // 押下状態にする
                }
                else if (m_pInput->ReturnMouse(0)) // 押下状態で左クリックが離されたかをチェック
                {
                    m_interactionState = InteractionState::Submitted; // 決定状態をリセット
                }
            }
            else
            {
                m_interactionState = InteractionState::Idle; // 通常状態にする
            }
            break;
        case ControlMode::GamePad:
            // ゲームパッド操作時の処理
            break;
    }
}

void Button::UpdateReactions() {
    switch (m_interactionState) // 現在の状態に応じたリアクションを更新
    {
    case InteractionState::Idle:
        if (m_interactionStatePre == InteractionState::Selected || m_interactionStatePre == InteractionState::Pressed)
        {
            if (m_onHoverReaction.highlight)
            {
                m_sprite->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // ハイライトをリセット
            }
            if (m_onHoverReaction.scale)
            {
                m_sprite->SetScale(m_originalSpriteSize); // スケールをリセット
            }
        }
        break;
    case InteractionState::Selected:
        if (m_onHoverReaction.custom)
        {
            m_onHoverReaction.custom();
        }
        if (m_onHoverReaction.highlight)
        {
            m_sprite->SetColor(m_onHoverReaction.highlightColor);
        }
        if (m_onHoverReaction.scale)
        {
            m_sprite->SetScale({ m_originalSpriteSize * m_onHoverReaction.scaleAmount });
        }
        break;
    case InteractionState::Pressed:
        if (m_onClickReaction.custom)
        {
            m_onClickReaction.custom();
        }
        if (m_onClickReaction.highlight)
        {
            m_sprite->SetColor(m_onClickReaction.highlightColor);
        }
        if (m_onClickReaction.scale)
        {
            m_sprite->SetScale({ m_originalSpriteSize * m_onClickReaction.scaleAmount });
        }
        break;
    case InteractionState::Submitted:
        if (m_onReleaseReaction.custom)
        {
            m_onReleaseReaction.custom();
        }
        if (m_onReleaseReaction.highlight)
        {
            m_sprite->SetColor(m_onReleaseReaction.highlightColor);
        }
        if (m_onReleaseReaction.scale)
        {
            m_sprite->SetScale({ m_originalSpriteSize * m_onReleaseReaction.scaleAmount });
        }
        if (m_activeReaction)
        {
            m_activeReaction(); // ボタンが押されたときのリアクションを呼び出す
        }
        break;
    }

}
