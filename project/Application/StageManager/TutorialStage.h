#include "BaseStage.h"
#include "Object3d.h"
#include "Sprite.h"
#include "Trap.h"
#include "Goal.h"

#pragma once
class TutorialStage : public BaseStage
{
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// JsonNameの取得
    /// </summary>
    virtual std::string GetJsonName();
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
    // ステージに必要なものの宣言
    std::unique_ptr<Object3d> stageObject;
    std::unique_ptr<Object3d> wallRunObject;
    std::unique_ptr<Trap> trap;
    std::unique_ptr<Goal> goal;

    std::array< std::unique_ptr<Sprite>, 4> tutorialSprites;

};

