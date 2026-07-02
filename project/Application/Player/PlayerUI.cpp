#include "PlayerUI.h"
#include "Player.h"
#include "WinApp.h"
#include "EngineContext.h"

void PlayerUI::Initialize(Player* player)
{
    AppContext& ctx = *m_pContext;
    m_player = player;

    Vector2 windowSize = ctx.engine.platform.window.GetWindowSize();

    m_climbingUI = ctx.game.spriteFactory.Create("Resources/Sprite/PlayerUI/Parkour_Climbing.png");
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

