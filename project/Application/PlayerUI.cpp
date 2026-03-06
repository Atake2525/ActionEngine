#include "PlayerUI.h"
#include "Player.h"
#include "WinApp.h"

void PlayerUI::Initialize(Player* player)
{
    m_player = player;

    Vector2 windowSize = WinApp::GetInstance()->GetWindowSize();

    m_climbingUI = std::make_unique<Sprite>();
    m_climbingUI->Initialize("Resources/Sprite/PlayerUI/Parkour_Climbing.png");
    m_climbingUI->SetAnchorPoint({ 0.5f, 1.0f });
    m_climbingUI->SetPosition({ windowSize.x * 0.5f, windowSize.y});
    m_climbingUI->SetColor({ 0.0f, 0.2f, 1.0f, 1.0f });
    m_climbingUI->Update();

}

void PlayerUI::Update()
{
    m_climbingUI->Update();
}

void PlayerUI::Draw()
{
    if (m_player->GetIsClimbing())
    {
        m_climbingUI->Draw();
    }
}

