#include "SceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Logger.h"

using namespace Logger;
using namespace std;


unique_ptr<BaseScene> SceneFactory::ChangeScene(const std::string& sceneName)
{
    // 次のシーンを生成
    unique_ptr<BaseScene> newScene = nullptr;

    if (sceneName == "TITLE")
    {
        Log("タイトルシーンに切り替えます\n");
        newScene = make_unique<TitleScene>();
        
    }
    else if (sceneName == "GAMESCENE")
    {
        Log("ゲームシーンに切り替えます\n");
        newScene = make_unique<GameScene>();
    }
    else
    {
        Log("指定されたシーン名は見つかりませんでした\n");
    }

    if (newScene) {
        newScene->SetContext(*m_pContext);
    }

    return newScene;
}
