#pragma once
#include "Sprite.h"
#include <memory>

class Player;

class PlayerUI
{
public:
    void Initialize(Player* player);

    void Update();

    void Draw();

private:
    Player* m_player;

    std::unique_ptr<Sprite> m_climbingUI;
};

