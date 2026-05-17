#include "UIButton.h"
#include "Collision.h"

using namespace UI;

UIButton::UIButton()
{}

UIButton::~UIButton()
{}

void UIButton::Initialize(const std::string& textureFilePath, Input& input) {
    // UI用のSpriteを作成して初期化する
    m_sprite = std::make_unique<Sprite>();
    m_sprite->Initialize(textureFilePath);
    m_originalSpriteSize = m_sprite->GetTextureSize();

    m_pInput = &input;
}

void UIButton::Update() {
    // ボタンが表示されている場合のみ更新する
    if (m_visible)
    {
        switch (m_transitionState)
        {
        case UITransitionState::Hidden: // ボタンが非表示の状態の処理
            break;
        case UITransitionState::Entering: // ボタンが表示される途中の処理
            m_enterReaction();
            break;
        case UITransitionState::Shown: // ボタンが表示されている状態の処理
            
            UpdateButtonState(); // ボタンの状態を更新する
            UpdateReactions(); // ボタンの状態に応じたリアクションを更新する

            // ボタンが押されたら指定されているアクティブリアクションを実行する
            if (m_buttonState == ButtonState::Released)
            {
                if (m_activeReaction)
                {
                }
            }

            break;
        case UITransitionState::Exiting: // ボタンが非表示になる途中の処理
            m_exitReaction();
            break;
        default:
            break;
        }

        m_sprite->Update();
    }
}

void UIButton::Draw() {
    // ボタンが表示されている場合のみ描画する
    if (m_visible)
    {
        m_sprite->Draw();
    }
}

void UIButton::UpdateButtonState() {
    // 前の状態を保存
    m_buttonStatePre = m_buttonState;

    // マウスの位置を取得
    Vector2 mousePos = m_pInput->GetWindowMousePos2();
    Vector2 buttonPos = m_sprite->GetPosition();
    Vector2 buttonSize = m_sprite->GetTextureSize();

    // マウスがボタンの上にあるかを確認
    if (CollisionUISprite(buttonPos, buttonSize, mousePos, { 1.0f, 1.0f }))
    {
        m_buttonState = ButtonState::Hovered; // ホバー状態にする
        if (m_pInput->PressMouse(0)) // 左クリックが押されているかをチェック
        {
            m_buttonState = ButtonState::Clicked; // クリック状態にする
        }
        else if (m_pInput->ReturnMouse(0)) // クリック状態で左クリックが離されたかをチェック
        {
            m_buttonState = ButtonState::Released; // クリック状態をリセット
        }
    }
    else
    {
        m_buttonState = ButtonState::Normal; // 通常状態にする
    }
}

void UIButton::UpdateReactions() {
    switch (m_buttonState)
    {
    case ButtonState::Normal:
        if (m_buttonStatePre == ButtonState::Hovered || m_buttonStatePre == ButtonState::Clicked)
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
    case ButtonState::Hovered:
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

        }
        break;
    case ButtonState::Clicked:
        break;
    case ButtonState::Released:
        break;
    default:
        break;
    }

}
