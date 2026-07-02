#include "EngineContext.h"
#pragma once

class Runtime
{
public:
    Runtime() = default;
    ~Runtime() = default;

    void Initialize();
    void Update();
    void Draw();

    const bool WindowProcessMessage();

    const AppContext& GetContext() const { return m_context; }
    const EngineContext& GetEngine() const { return m_context.engine; }
    const WorldContext& GetWorld() const { return m_context.world; }
    const GameContext& GetGame() const { return m_context.game; }
    bool EndRequest() const { return m_loopOut; }

private:
    AppContext m_context;
    bool m_loopOut = false;

    void SetupEngine();
    void SetupWorld();
    void SetupGame();
};

