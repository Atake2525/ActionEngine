#include "BaseScene.h"
#include "SceneFactory.h"
#include <memory>
#include "SettingManager.h"

#pragma once
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void SetContext(AppContext& context) { m_pContext = &context; }
    // 次シーン予約
    void SetNextScene(const std::string& sceneName);

    const std::string& GetSceneName() const { return m_sceneName; }

    SettingManager& GetSettingManager() { return m_settingManager; }

    void Update();

    void Draw();

    void CallStart();

    const bool& EndRequest() { return m_loopOut; }

private:
    AppContext* m_pContext;

    bool drawStart = false;
    // 実行中のシーン
    std::unique_ptr<BaseScene> m_scene = nullptr;
    // 次のシーン
    std::unique_ptr<BaseScene> m_nextScene = nullptr;

    SettingManager m_settingManager;

    std::string m_sceneName = "0";

    bool m_loopOut = false;
};

