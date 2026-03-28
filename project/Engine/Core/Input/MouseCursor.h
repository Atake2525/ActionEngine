#include "Sprite.h"
#include <string>
#include <memory>
#include <array>
#pragma once

class Input;

class MouseCursor
{
public:
    MouseCursor();
    ~MouseCursor();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// /// <param name="hover">hovre状態</param>
    /// <param name="press">press状態</param>
    void Initialize(const std::string& hover, const std::string& press);
    /// <summary>
    /// カーソルのテクスチャのファイルパスを指定
    /// </summary>
    /// <param name="hover">hovre状態</param>
    /// <param name="press">press状態</param>
    void SetCursorTextureFilePath(const std::string& hover, const std::string& press);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="注意">SpriteBase->ShaderDrawの後で実行すること</param>
    void Draw();

    void SetShowCursor(bool flag) { m_isShowCursor = flag; }

    const Vector2& GetCursorPos() const { return m_cursorPos; }

    void SetCursorPosition(const Vector2& mousePosition);

private:
    // マウスカーソルのテクスチャ群
    std::array<std::string, 2> m_cursorTextures;
    // テクスチャを張るためのスプライトクラス
    std::unique_ptr<Sprite> m_cursorSprite;
    // マウスの押下を調べるためのInputクラス
    Input* m_input = nullptr;
    // マウスカーソルの位置
    Vector2 m_cursorPos = Vector2::Zero;
    // ウィンドウサイズ(画面外にカーソルが出ないように)
    Vector2 m_windowSize = Vector2::Zero;
    // マウスカーソルを表示するかどうか
    bool m_isShowCursor = true;
};


