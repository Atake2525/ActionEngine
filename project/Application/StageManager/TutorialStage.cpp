#include "TutorialStage.h"
#include "JsonLoader.h"
#include "WinApp.h"
#include "CollisionManager.h"

using namespace std;

void TutorialStage::Initialize()
{
    JsonLoader::GetInstance()->LoadJson("Resources/Json/Stage/Tutorial.json", "TutorialStage", false);

    stageObject = make_unique<Object3d>();
    stageObject->Initialize();
    stageObject->SetModel("Resources/Model/obj/Stage/TutorialStage", "TutorialStage.obj", true);
    CollisionManager::GetInstance()->AddCollision(stageObject.get(), "stageObject");

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
}

std::string TutorialStage::GetJsonName()
{
    return "TutorialStage";
}

void TutorialStage::Update()
{
    stageObject->Update();
}

void TutorialStage::DrawObject3d()
{
    stageObject->Draw();
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
    CollisionManager::GetInstance()->DeleteCollision("stageObject");
    JsonLoader::GetInstance()->DeleteJson("TutorialStage");
}
