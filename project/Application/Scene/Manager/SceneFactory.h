#pragma once

#include "AbstractSceneFactory.h"

class SceneFactory : public AbstractSceneFactory
{
public:

    void SetContext(AppContext& context) { m_pContext = &context; }

    void CallStart();

    /// <summary>
    /// シーン生成
    /// </summary>
    /// <param name="sceneName">シーン名</param>
    /// <returns></returns>
    std::unique_ptr<BaseScene> ChangeScene(const std::string& sceneName) override;

private:
    AppContext* m_pContext = nullptr;
};

