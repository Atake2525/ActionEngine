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
    AppContext& context = *m_pContext;
    context.engine.graphics.offScreen.SetGrayscaleColor(GRAYSCALE_SEPIA);

    context.engine.assets.json.LoadJson("Resources/Json/Stage/Tutorial3.json", "TutorialStage", false);

    // ステージオブジェクトの初期化
    stageObject = context.game.object3dFactory.Create();
    Model* model = context.engine.assets.models.LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Model.obj");
    stageObject->SetModel(model);
    stageObject->SetEnableLighting(true);
    stageObject->Update();

    collisionObject = context.game.object3dFactory.Create();
    model = context.engine.assets.models.LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Collision.obj", false);
    collisionObject->SetModel(model);
    collisionObject->Update();
    
    context.world.collision.AddCollision(collisionObject.get());

    // トラップの初期化
    trap = make_unique<Trap>();
    trap->SetContext(context);
    trap->Initialize("TutorialStage");

    // ゴールの初期化
    goal = make_unique<Goal>();
    goal->SetContext(context);
    goal->Initialize("TutorialStage", player);

    float windowSizeX = float(context.engine.platform.window.GetkClientWidth());
    for (int i = 0; i < 4; i++)
    {
        tutorialSprites[i] = context.game.spriteFactory.Create("Resources/Sprite/tutorialUI/tutorial" + to_string(i + 1) + ".png");
        tutorialSprites[i]->SetAnchorPoint({ 1.0f, 0.0f });
        tutorialSprites[i]->SetPosition({ windowSizeX, 50.0f * i });
        tutorialSprites[i]->SetScale({ 300.0f, 50.0f });
        tutorialSprites[i]->Update();
    }


    m_player = player;
    m_camera = camera;
}

std::string TutorialStage::GetJsonName()
{
    return "TutorialStage";
}

void TutorialStage::Update()
{
    AppContext& context = *m_pContext;
#ifndef NDEBUG
    if (context.engine.platform.input.TriggerKey(DIK_RETURN))
    {
        goal->SetGoal();
    }
#endif // !NDEBUG

    if (m_player->GetTransform().position.y < -120.0f)
    {
        context.engine.presentation.fade.FadeOut(0.5f);

        std::function func = [this]() {
            m_pContext->engine.graphics.offScreen.SetGrayscaleIntensity(0.0f);
            m_pContext->game.sceneManager.SetNextScene("GAMESCENE");
            };

        context.engine.presentation.fade.SetFinishedFadeFunction(func);
    }

    stageObject->Update();

    trap->Update();

    goal->Update();
}

void TutorialStage::DrawObject3d()
{
    stageObject->Draw();
    trap->Draw();
    goal->DrawGoalObject();
    for (int i = 0; i < 4; i++)
    {
        tutorialSprites[i]->Update();
    }
}

void TutorialStage::DrawFrontSprite() {}

void TutorialStage::DrawBackSprite()
{
    goal->DrawResult();
}

void TutorialStage::Finalize()
{
    AppContext& context = *m_pContext;
    if (collisionObject)
    {
        context.world.collision.DeleteCollision(collisionObject.get());
    }
    if (wallRunObject)
    {
        context.world.collision.DeleteCollision(wallRunObject.get());
        context.world.collision.DeleteWallDashCollision(wallRunObject.get());
    }
    context.engine.assets.json.DeleteJson("TutorialStage");
}
