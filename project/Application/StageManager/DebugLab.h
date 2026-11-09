#include "BaseStage.h"
#include "Object3d.h"
#include "Sprite.h"
#include "MoveObject.h"
#include "Goal.h"
#pragma once

class DebugLab : public BaseStage
{
public:
    void SetContext(AppContext& context) override;

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(Player* player, Camera* camera) override;
    /// <summary>
    /// JsonNameの取得
    /// </summary>
    std::string GetJsonName() override;
    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;
    /// <summary>
    /// 3Dオブジェクトの描画処理
    /// </summary>
    void DrawObject3d() override;
    /// <summary>
    /// 前景スプライトの描画処理
    /// </summary>
    void DrawFrontSprite() override;
    /// <summary>
    /// 背景スプライトの描画処理
    /// </summary>
    void DrawBackSprite() override;
    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

private:
    Player* m_player = nullptr;
    Camera* m_camera = nullptr;
    // ステージに必要なものの宣言
    std::unique_ptr<Object3d> m_stageObject;
    std::unique_ptr<Object3d> m_collisionObject;
    std::unique_ptr<MoveObject> m_moveObject;
    std::unique_ptr<Goal> m_pGoal;
};

