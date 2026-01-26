#include "GameScene.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "GameTime.h"
#include "JsonLoader.h"
#include "GameOver.h"
#include "StageCount.h"
#include "TutorialStage.h"

using namespace std;
using namespace ActionEngine::Stage;

void GameScene::Initialize() {

    TextureManager::GetInstance()->LoadTexture("Resources/rostock_laage_airport_4k.dds");

    camera = make_unique<Camera>();
    camera->SetTranslate({ 0.0f, 1.8f, 0.0f });

    SkyBox::GetInstance()->SetCamera(camera.get());
    SkyBox::GetInstance()->SetTexture("Resources/rostock_laage_airport_4k.dds");

    input = Input::GetInstance();

    gameOver_ = make_unique<GameOver>();
    gameOver_->Initialize();
    gameClear_ = make_unique<GameClearScene>();
    gameClear_->Initialize();


    Object3dBase::GetInstance()->SetDefaultCamera(camera.get());

    stage = make_unique<TutorialStage>();
    stage->Initialize();

    stageObject = make_unique<Object3d>();
    stageObject->Initialize();
    stageObject->SetModel("Resources/Model/obj/Stage/TutorialStage", "TutorialStage.obj", true);

    player_ = make_unique<ActionPlayer>();
    player_->Initialize(camera.get(), stage->GetJsonName());

    GameTime::GetInstance()->SetDeltaPoint();
    FadeManager::GetInstance()->FadeIn(1.0f);
}

void GameScene::Update() {


    stage->Update();

    //input->Update();

    stageObject->Update();

    player_->Update();

    if (input->TriggerKey(DIK_ESCAPE))
    {
        finished = true;
    }

    if (input->TriggerKey(DIK_F11))
    {
        cursorshow = !cursorshow;
    }
    input->ShowMouseCursor(cursorshow);

    if (input->TriggerKey(DIK_1))
    {
        Audio::GetInstance()->Play2D("bgm", { 0.0f, 0.0f }, false);
    }


    SkyBox::GetInstance()->Update();

   /* if (!start_)
    {
        if (FadeManager::GetInstance()->CompleteFade())
        {
            start_ = true;
            startMovie_ = true;
            phase_ = 0;
        }
        else
        {
            return;
        }
    }*/

    if (input->TriggerKey(DIK_R))
    {
        SceneManager::GetInstance()->SetNextScene("GAMESCENE");
    }

    /*if (input->PushKey(DIK_LSHIFT))
    {
        tutorial[1].timer += GameTime::GetInstance()->GetDeltaTime();
        tutorial[1].color.x = 1.0f - (tutorial[1].timer / 2.0f);
        tutorial[1].color.z = 1.0f - (tutorial[1].timer / 2.0f);
        tutorial[1].sprite->SetColor(tutorial[1].color);
        if (tutorial[1].timer >= 2.0f)
        {
            tutorial[1].isClear = true;
        }
        tutorial[1].sprite->Update();

    }

    if (player_->IsGameOver())
    {
        player_->Freeze(true);
        gameOver_->Update();
    }*/

    camera->Update();

}

void GameScene::Draw() {

    SpriteBase::GetInstance()->ShaderDraw();


    Object3dBase::GetInstance()->ShaderDraw();

    stage->DrawObject3d();
    stageObject->Draw();

    SkinningObject3dBase::GetInstance()->ShaderDraw();

    //player_->Draw();

    SpriteBase::GetInstance()->ShaderDraw();

    //stage->DrawBackSprite();
    gameOver_->Draw();
    gameClear_->Draw();
}

void GameScene::Finalize() {
}