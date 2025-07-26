#include "SceneManager.h"

SceneManager* SceneManager::instance = nullptr;

void SceneManager::Finalize() {
    nextScene_ = nullptr;
    scene_->Finalize();
    delete scene_;

    SceneFactory::GetInstance()->Finalize();
    delete instance;
    instance = nullptr;
}

SceneManager* SceneManager::GetInstance() {
    SceneFactory::GetInstance();
    if (instance == nullptr)
    {
        instance = new SceneManager;
    }
    return instance;
}

void SceneManager::SetNextScene(const std::string& sceneName)
{
    nextScene_ = SceneFactory::GetInstance()->ChangeScene(sceneName);
}

void SceneManager::Update() {
    // 次のシーン予約があるなら
    if (nextScene_) {
        // 旧シーンの終了
        if (scene_)
        {
            scene_->Finalize();
            delete scene_;
        }

        // シーン切り替え
        scene_ = nextScene_;
        nextScene_ = nullptr;

        scene_->SetSceneManager(this);

        // 次のシーンを初期化する
        scene_->Initialize();
    }
    scene_->Update();

    if (scene_->EndRequest())
    {
        roopOut_ = true;
    }
}

void SceneManager::Draw() {
    if (scene_)
    {
        scene_->Draw();
    }
}