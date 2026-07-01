#include <memory>
#include <string>

class Player;
class Camera;
struct AppContext;

#pragma once
class BaseStage
{
public:

    virtual ~BaseStage() = default;

    virtual void SetContext(AppContext& context) { m_pContext = &context; }

    /// <summary>
    /// 初期化
    /// </summary>
    virtual void Initialize(Player* player, Camera* camera);

    /// <summary>
    /// JsonNameの取得
    /// </summary>
    virtual std::string GetJsonName();

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Update();
    
    /// <summary>
    /// 3Dオブジェクトの描画処理
    /// </summary>
    virtual void DrawObject3d();

    /// <summary>
    /// 前景スプライトの描画処理
    /// </summary>
    virtual void DrawFrontSprite();

    /// <summary>
    /// 背景スプライトの描画処理
    /// </summary>
    virtual void DrawBackSprite();

    /// <summary>
    /// 終了処理
    /// </summary>
    virtual void Finalize();

protected:
    AppContext* m_pContext = nullptr;

private:
    Player* m_player = nullptr;
    Camera* m_camera = nullptr;
};

