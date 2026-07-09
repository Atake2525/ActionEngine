#include "SceneManager.h"
#include "GameTime.h"
#include "EngineContext.h"

using namespace std;

SceneManager::SceneManager() = default;

SceneManager::~SceneManager() {
    m_nextScene = nullptr;
    if (m_scene) {
        m_scene->Finalize();
    }
}

void SceneManager::SetNextScene(const std::string& sceneName)
{
    m_sceneName = sceneName;
    drawStart = false;
    m_nextScene = m_pContext->game.sceneFactory.ChangeScene(sceneName);
}

void SceneManager::Update() {
    // 次のシーン予約があるなら
    if (m_nextScene) {
        // 旧シーンの終了
        if (m_scene)
        {
            m_scene->Finalize();
        }

        // シーン切り替え
        m_scene = move(m_nextScene);
        m_nextScene = nullptr;

        m_scene->SetSceneManager(this);

        // 次のシーンを初期化する
        m_scene->Initialize();
    }
    m_scene->Update();

    if (m_scene->EndRequest())
    {
        m_loopOut = true;
    }
}

void SceneManager::Draw() {
    if (m_scene)
    {
        m_scene->Draw();
    }
}

void SceneManager::CallStart()
{
    if (m_scene && !drawStart)
    {
        drawStart = true;
        m_pContext->engine.platform.time.SetDeltaPoint();
    }
}
