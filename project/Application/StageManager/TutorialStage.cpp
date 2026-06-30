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

using namespace std;

void TutorialStage::Initialize(Player* player, Camera* camera)
{
    OffScreenRendering::GetInstance()->SetGrayscaleColor(GRAYSCALE_SEPIA);

    JsonLoader::GetInstance()->LoadJson("Resources/Json/Stage/Tutorial3.json", "TutorialStage", false);

    // ステージオブジェクトの初期化
    stageObject = make_unique<Object3d>();
    stageObject->Initialize();
    Model* model = ModelManager::GetInstance()->LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Model.obj");
    stageObject->SetModel(model);
    stageObject->SetEnableLighting(true);
    stageObject->Update();

    collisionObject = make_unique<Object3d>();
    collisionObject->Initialize();
    model = ModelManager::GetInstance()->LoadModel("Resources/Model/obj/Stage/TutorialStage3", "Collision.obj", false);
    collisionObject->SetModel(model);
    collisionObject->Update();
    
    CollisionManager::GetInstance()->AddCollision(collisionObject.get());

    // トラップの初期化
    trap = make_unique<Trap>();
    trap->Initialize("TutorialStage");

    // ゴールの初期化
    goal = make_unique<Goal>();
    goal->Initialize("TutorialStage", player);

    float windowSizeX = float(WinApp::GetInstance()->GetkClientWidth());
    for (int i = 0; i < 4; i++)
    {
        tutorialSprites[i] = make_unique<Sprite>(); 
        tutorialSprites[i]->Initialize("Resources/Sprite/tutorialUI/tutorial" + to_string(i + 1) + ".png");
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
#ifndef NDEBUG
    if (Input::GetInstance()->TriggerKey(DIK_RETURN))
    {
        goal->SetGoal();
    }
#endif // !NDEBUG

    if (m_player->GetTransform().position.y < -120.0f)
    {
        FadeManager::GetInstance()->FadeOut(0.5f);

        std::function func = [this]() {
            OffScreenRendering::GetInstance()->SetGrayscaleIntensity(0.0f);
            SceneManager::GetInstance()->SetNextScene("GAMESCENE");
            };

        FadeManager::GetInstance()->SetFinishedFadeFunction(func);
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
    if (collisionObject)
    {
        CollisionManager::GetInstance()->DeleteCollision(collisionObject.get());
    }
    if (wallRunObject)
    {
        CollisionManager::GetInstance()->DeleteCollision(wallRunObject.get());
        CollisionManager::GetInstance()->DeleteWallDashCollision(wallRunObject.get());
    }
    JsonLoader::GetInstance()->DeleteJson("TutorialStage");
}
