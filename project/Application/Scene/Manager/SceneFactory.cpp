#include "SceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Logger.h"

using namespace Logger;

SceneFactory* SceneFactory::instance = nullptr;

void SceneFactory::Finalize() {
    delete instance;
    instance = nullptr;
}

SceneFactory* SceneFactory::GetInstance() {
    if (instance == nullptr)
    {
        instance = new SceneFactory;
    }
    return instance;
}

BaseScene* SceneFactory::ChangeScene(const std::string& sceneName)
{
    // 次のシーンを生成
    BaseScene* newScene = nullptr;

    if (sceneName == "TITLE")
    {
        Log("タイトルシーンに切り替えます\n");
        newScene = new TitleScene();
    }
    else if (sceneName == "GAMESCENE")
    {
        Log("ゲームシーンに切り替えます\n");
        newScene = new GameScene();
    }
    else
    {
        Log("指定されたシーン名は見つかりませんでした\n");
    }


    return newScene;
}
