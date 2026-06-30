#include "EngineContext.h"
#pragma once

class Runtime
{
public:
    Runtime();
    ~Runtime();

    void Initialize();
    void Update();
    void Draw();

    const bool WindowProcessMessage();

    const AppContext& GetContext() const { return m_context; }
    const EngineContext& GetEngine() const { return m_context.engine; }
    const WorldContext& GetWorld() const { return m_context.world; }
    const GameContext& GetGame() const { return m_context.game; }

private:
    AppContext m_context;

    void SetupEngine();
    void SetupWorld();
    void SetupGame();
};

