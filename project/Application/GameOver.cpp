#include "GameOver.h"
#include "GameTime.h"
#include "EasingUtility.h"
#include "TextureManager.h"
#include "OffScreenRnedering.h"
#include "WinApp.h"
#include "FadeManager.h"
#include "SceneManager.h"

using namespace std;

void GameOver::Initialize() {
    TextureManager::GetInstance()->LoadTexture("Resources/Sprite/UI/gameOver.png");

    backScreen = make_unique<Sprite>();
    backScreen->Initialize("Resources/Sprite/UI/gameOver.png");
    backScreen->SetAnchorPoint({ 0.5f, 0.5f });
    backScreen->SetPosition({ 640.0f, 360.0f });
    backScreen->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    backScreen->Update();

    frame = make_unique<Sprite>();
    frame->Initialize("Resources/Debug/black1x1.png");
    frame->SetScale({ float(WinApp::GetInstance()->GetkClientWidth()), float(WinApp::GetInstance()->GetkClientHeight()) });
    frame->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    frame->Update();

    space = make_unique<Sprite>();
    space->Initialize("Resources/Sprite/UI/pushSpace.png");
    space->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    space->SetAnchorPoint({ 0.5f, 0.5f });
    space->SetPosition({ 640.0f, 360.0f });
    space->Update();

    restart = make_unique<UI>();
    stageSelect = make_unique<UI>();
    title = make_unique<UI>();

    input = Input::GetInstance();
}

void GameOver::Update() {

    float easeAlpha;
    switch (animationNumber)
    {
    case 0:
        moveTimer_ += GameTime::GetInstance()->GetDeltaTime() / moveTime_;

        easeAlpha = EaseOutQuint(moveTimer_, 0.0f, 1.0f);

        frame->SetColor({ 1.0f, 1.0f, 1.0f, easeAlpha });
        frame->Update();
        backScreen->SetColor({ 1.0f, 1.0f, 1.0f, easeAlpha });
        backScreen->Update();
        if (moveTimer_ > 1.0f)
        {
            animationNumber++;
            moveTimer_ = 0.0f;
        }
        break;

    case 1:
        moveTimer_ += GameTime::GetInstance()->GetDeltaTime();

        easeAlpha = EaseOutQuint(moveTimer_, 0.0f, 1.0f);

        space->SetColor({ 1.0f, 1.0f, 1.0f, easeAlpha });
        space->Update();
        if (moveTimer_ > 1.0f)
        {
            animationNumber++;
            moveTimer_ = 0.0f;
        }
        break;

    default:
        break;
    }

    if (animationNumber < 2)
    {
        return;
    }

    if (input->PushKey(DIK_SPACE))
    {
        FadeManager::GetInstance()->FadeOut(1.0f);
        push = true;
    }
    if (push && FadeManager::GetInstance()->CompleteFade())
    {
        SceneManager::GetInstance()->SetNextScene("GAMESCENE");
    }

    frame->Update();
    backScreen->Update();
    space->Update();
}

void GameOver::Draw() {
    frame->Draw();
    backScreen->Draw();
    space->Draw();
}