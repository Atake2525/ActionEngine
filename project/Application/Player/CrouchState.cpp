#include "CrouchState.h"
#include "SlidingState.h"
#include "Player.h"

void CrouchState::Enter(Player& player) {
	if (player.m_playerSpeed / player.m_delta >= player.m_runSpeed * player.m_maxSlideSpeedThreshold)
	{
		player.ChangeState(std::make_unique<SlidingState>());
	}
}

void CrouchState::Update(Player& player) {
    player.HorizontalMove(player.m_crounchSpeed, player.m_crouchAccelerationTime, player.m_groundDecelerationTime);
}

void CrouchState::Exit(Player& player) {

}