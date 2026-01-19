#include "SceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "TestScene.h"
#include "StageSelectScene.h"
#include "Logger.h"

using namespace Logger;
using namespace std;

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
    else if (sceneName == "TEST") {
        Log("テストシーンに切り替えます\n");
        newScene = make_unique<TestScene>();
    }
    else if (sceneName == "STAGESELECT")
    {
        Log("ステージセレクトシーンに切り替えます\n");
        newScene = make_unique<StageSelectScene>();
    }
    else
    {
        Log("指定されたシーン名は見つかりませんでした\n");
    }


    return newScene;
}
