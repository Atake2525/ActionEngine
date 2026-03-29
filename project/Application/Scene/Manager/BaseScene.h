#pragma once
#include "FadeManager.h"

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

    virtual void SetSceneManager(SceneManager* sceneManager) { m_sceneManager = sceneManager; }

private:
    bool m_endRequest = false;

    bool m_drawStart = false;

    // シーンマネージャ
    SceneManager* m_sceneManager = nullptr;
};

