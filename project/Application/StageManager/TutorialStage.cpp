#include "TutorialStage.h"
#include "JsonLoader.h"
#include "WinApp.h"
#include "CollisionManager.h"

using namespace std;

void TutorialStage::Initialize()
{
    JsonLoader::GetInstance()->LoadJson("Resources/Json/Stage/Tutorial.json", "TutorialStage", false);

    // ステージオブジェクトの初期化
    stageObject = make_unique<Object3d>();
    stageObject->Initialize();
    stageObject->SetModel("Resources/Model/obj/Stage/TutorialStage", "TutorialStage.obj", true);
    CollisionManager::GetInstance()->AddCollision(stageObject.get());

    // トラップの初期化
    trap = make_unique<Trap>();
    trap->Initialize("t");

    // ゴールの初期化
    goal = make_unique<Goal>();
    goal->Initialize("t");

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

    trap->Update();

   /* if (startMovie_)
    {
        movieTimer_ += GameTime::GetInstance()->GetDeltaTime();

        float farClip = camera->GetFarClipDistance();
        float height;

        AABB landAABB = land->GetAABB();
        Vector3 landSize = landAABB.max - landAABB.min;
        float flatLandSize = landSize.x * landSize.z;

        switch (phase_)
        {
        case 0:

            farClip = Lerp(1.0f, flatLandSize, movieTimer_ / movieTime_);

            camera->SetFarClipDistance(farClip);
            break;
        case 1:
            height = Lerp(-1.0f, landAABB.max.y + 10.0f, movieTimer_ / movieTime_);
            land->SetDrawHeiht(height);
            trap_->SetDrawHeight(height);
            break;
        }

        if (movieTimer_ >= movieTime_)
        {
            if (phase_ == 1)
            {
                startMovie_ = false;
                movieTimer_ = 0.0f;
                player_->Freeze(false);
            }
            else
            {
                camera->SetFarClipDistance(100.0f);
                movieTimer_ = 0.0f;
                phase_++;
                return;
            }
        }
        else
        {
            return;
        }
    }*/

}

void TutorialStage::DrawObject3d()
{
    stageObject->Draw();
    trap->Draw();
    goal->Draw();
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
    JsonLoader::GetInstance()->DeleteJson("TutorialStage");
}
