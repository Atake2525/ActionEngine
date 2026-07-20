#include "SlidingState.h"
#include "CrouchState.h"
#include "Player.h"

void SlidingState::Enter(Player& player) {

}

void SlidingState::Update(Player& player) {
    bool complete = player.HorizontalMove(player.m_crounchSpeed, player.m_slidingAccelerationTime, player.m_slidingAccelerationTime);
	if (complete)
	{
		player.ChangeState(std::make_unique<CrouchState>());
	}
}

void SlidingState::Exit(Player& player) {

}