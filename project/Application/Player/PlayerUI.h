#pragma once
#include "Sprite.h"
#include <memory>

class Player;
struct AppContext;

class PlayerUI
{
public:
    void SetContext(AppContext& context) { m_pContext = &context; }

    void Initialize(Player* player);

    void Update();

    void Draw();

private:
    AppContext* m_pContext = nullptr;
    Player* m_player;

    std::unique_ptr<Sprite> m_climbingUI;
};

