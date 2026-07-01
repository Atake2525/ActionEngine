#pragma once
#include "FadeManager.h"
#include "Runtime.h"

class SceneManager;

class BaseScene
{
public:

    virtual ~BaseScene() = default;

    virtual void Initialize();

    virtual void Update();

    virtual void Draw();

    virtual void Finalize();

    virtual const bool& EndRequest() { return m_endRequest; }

    virtual void SetContext(AppContext* context) { m_pContext = context; }

protected:
    bool m_endRequest = false;

    bool m_drawStart = false;

    // シーンマネージャ
    AppContext* m_pContext = nullptr;
};

