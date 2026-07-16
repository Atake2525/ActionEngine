#include "TutorialStage.h"
#include "JsonLoader.h"
#include "WinApp.h"
#include "CollisionManager.h"
#include "Light.h"
#include "Player.h"
#include "OffscreenRendering.h"
#include "FadeManager.h"
#include <functional>
#include "SceneManager.h"
#include "ModelManager.h"
#include "EngineContext.h"

using namespace std;

void TutorialStage::SetContext(AppContext& context)
{
    BaseStage::SetContext(context);
}

void TutorialStage::Initialize(Player* player, Camera* camera)
{
    AppContext& ctx = *m_pContext;
    ctx.engine.graphics.offScreen.SetGrayscaleColor(GRAYSCALE_SEPIA);

    ctx.engine.assets.json.LoadJson("Resources/Json/Stage/Tutorial3.json", "TutorialStage", false);

    // ステージオブジェクトの初期化
    m_pStageObject = ctx.game.object3dFactory.Create();
    Model* model = ctx.engine.assets.models.LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Model.obj");
    m_pStageObject->SetModel(model);
    m_pStageObject->SetEnableLighting(true);
    m_pStageObject->Update();

    m_pCollisionObject = ctx.game.object3dFactory.Create();
    model = ctx.engine.assets.models.LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Collision.obj", false);
    m_pCollisionObject->SetModel(model);
    m_pCollisionObject->Update();
    
    ctx.world.collision.AddCollision(m_pCollisionObject.get());

    // トラップの初期化
    m_pMoveObject = make_unique<MoveObject>();
    m_pMoveObject->SetContext(ctx);
    m_pMoveObject->Initialize("TutorialStage");

    // ゴールの初期化
    m_pGoal = make_unique<Goal>();
    m_pGoal->SetContext(ctx);
    m_pGoal->Initialize("TutorialStage", player);

    m_player = player;
    m_camera = camera;
}

std::string TutorialStage::GetJsonName()
{
    return "TutorialStage";
}

void TutorialStage::Update()
{
    AppContext& ctx = *m_pContext;
#ifndef NDEBUG
    if (ctx.engine.platform.input.TriggerKey(DIK_RETURN))
    {
        m_pGoal->SetGoal();
    }
#endif // !NDEBUG

    if (m_player->GetTransform().position.y < -120.0f)
    {
        ctx.engine.presentation.fade.FadeOut(0.5f);

        std::function func = [this]() {
            m_pContext->engine.graphics.offScreen.SetGrayscaleIntensity(0.0f);
            m_pContext->game.sceneManager.SetNextScene("GAMESCENE");
            };

        ctx.engine.presentation.fade.SetFinishedFadeFunction(func);
    }

    m_pStageObject->Update();

    m_pMoveObject->Update();

    m_pGoal->Update();
}

void TutorialStage::DrawObject3d()
{
    m_pStageObject->Draw();
    m_pMoveObject->Draw();
    m_pGoal->DrawGoalObject();
}

void TutorialStage::DrawFrontSprite() {}

void TutorialStage::DrawBackSprite()
{
    m_pGoal->DrawResult();
}

void TutorialStage::Finalize()
{
    AppContext& ctx = *m_pContext;
    if (m_pCollisionObject)
    {
        ctx.world.collision.DeleteCollision(m_pCollisionObject.get());
    }
    if (m_pWallRunObject)
    {
        ctx.world.collision.DeleteCollision(m_pWallRunObject.get());
    }
    ctx.engine.assets.json.DeleteJson("TutorialStage");
}
