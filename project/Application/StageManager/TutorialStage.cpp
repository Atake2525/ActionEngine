#include "TutorialStage.h"
#include "JsonLoader.h"
#include "WinApp.h"
#include "CollisionManager.h"
#include "Player.h"
#include "OffscreenRendering.h"

using namespace std;

void TutorialStage::Initialize(Player* player, Camera* camera)
{
    OffScreenRendering::GetInstance()->SetGrayscaleColor(GRAYSCALE_SEPIA);

    JsonLoader::GetInstance()->LoadJson("Resources/Json/Stage/Tutorial.json", "TutorialStage", false);

    // ステージオブジェクトの初期化
    stageObject = make_unique<Object3d>();
    stageObject->Initialize();
    stageObject->SetModel("Resources/Model/obj/Stage/TutorialStage", "TutorialStage.obj", true);
    CollisionManager::GetInstance()->AddCollision(stageObject.get());
    stageObject->Update();

    wallRunObject = make_unique<Object3d>();
    wallRunObject->Initialize();
    wallRunObject->SetModel("Resources/Model/obj/Stage/TutorialStage", "TutorialStageWallDashObject.obj", true);
    CollisionManager::GetInstance()->AddCollision(wallRunObject.get());
    CollisionManager::GetInstance()->AddWallDashColliison(wallRunObject.get());

    wallRunObject->SetColor({ 1.0f, 0.5f, 0.5f, 1.0f });
    wallRunObject->Update();

    // トラップの初期化
    trap = make_unique<Trap>();
    trap->Initialize("t");

    // ゴールの初期化
    goal = make_unique<Goal>();
    goal->Initialize("t", player);

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
    stageObject->Update();

    wallRunObject->Update();

    trap->Update();

    goal->ChceckIsGoal();
}

void TutorialStage::DrawObject3d()
{
    stageObject->Draw();
    wallRunObject->Draw();
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
    for (int i = 0; i < 4; i++)
    {
        tutorialSprites[i]->Draw();
    }
}

void TutorialStage::Finalize()
{
    CollisionManager::GetInstance()->DeleteCollision(stageObject.get());
    CollisionManager::GetInstance()->DeleteCollision(wallRunObject.get());
    CollisionManager::GetInstance()->DeleteWallDashCollision(wallRunObject.get());
    JsonLoader::GetInstance()->DeleteJson("TutorialStage");
}
