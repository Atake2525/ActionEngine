#pragma once

class SceneManager;

class BaseScene
{
public:

    virtual ~BaseScene() = default;

    virtual void Initialize();

    virtual void Update();

    virtual void Draw();

    virtual void Finalize();

    virtual const bool& EndRequest() { return endRequest; }

    virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

private:
    bool endRequest = false;

    // シーンマネージャ
    SceneManager* sceneManager_ = nullptr;
};

