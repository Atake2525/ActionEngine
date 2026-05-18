#include <memory>
#include <functional>
#include "Sprite.h"
#include "Input.h"

#pragma once

// UIの状態を管理するクラス
enum class UITransitionState {
    Hidden = 0, // 非表示
    Entering,   // 表示中
    Shown,      // 表示
    Exiting     // 非表示にする途中
};

class UIElement
{
public:
    ~UIElement() = default;

	virtual void Initialize(const std::string& textureFilePath, Input& input) = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;

    void SetEnterReaction(const std::function<void()>& reaction) { m_enterReaction = reaction; }
    void SetExitReaction(const std::function<void()>& reaction) { m_exitReaction = reaction; }
    void SetTransitionState(UITransitionState state) { m_transitionState = state; }

    void ShowThisFrame() { m_transitionState = UITransitionState::Shown; }
    void Show() { m_transitionState = UITransitionState::Entering; }
    void Hide() { m_transitionState = UITransitionState::Exiting; }

    void SetPosition(const Vector2& pos);
    const Vector2& GetPosition() const;
    void SetScale(const Vector2& scale);
    const Vector2& GetScale() const;

protected:
    Input* m_pInput = nullptr;
	std::unique_ptr<Sprite> m_sprite;
    std::function<void()> m_enterReaction;
    std::function<void()> m_exitReaction;
    UITransitionState m_transitionState = UITransitionState::Hidden;
};