#include "GameOver.h"
#include "GameTime.h"
#include "EasingUtility.h"
#include "TextureManager.h"

using namespace std;

void GameOver::Initialize() {
    TextureManager::GetInstance()->LoadTexture("Resources/Sprite/UI/gameOver.png");

    backScreen = make_unique<Sprite>();
    backScreen->Initialize("Resources/Sprite/UI/gameOver.png");
    backScreen->SetAnchorPoint({ 0.5f, 0.5f });
    backScreen->SetPosition({ 640.0f, 360.0f });
    //backScreen->SetScale({ 0.0f, 0.0f });
    backScreen->Update();
    frame = make_unique<Sprite>();


    restart = make_unique<UI>();
    stageSelect = make_unique<UI>();
    title = make_unique<UI>();

    input = Input::GetInstance();
}

void GameOver::Update() {

    if (!canSelect)
    {
        moveTimer_ += GameTime::GetInstance()->GetDeltaTime() / moveTime_;

        Vector2 easeScale = EaseOutQuint(moveTimer_, Vector2{ 0.0f, 0.0f }, { 480.0f, 260.0f });

        backScreen->SetScale(easeScale);
        backScreen->Update();
        if (moveTimer_ > 1.0f)
        {
            canSelect = true;
            moveTimer_ = 0.0f;
        }

        return;
    }

    backScreen->Update();
}

void GameOver::Draw() {
    backScreen->Draw();
}