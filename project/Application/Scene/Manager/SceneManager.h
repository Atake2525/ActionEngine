#include "BaseScene.h"
#include "SceneFactory.h"

#pragma once
class SceneManager {
private:

    // コンストラクタ、デストラクタの隠蔽
    SceneManager() = default;
    ~SceneManager() = default;
    // コピーコンストラクタ、コピー代入演算子の封印
    SceneManager(SceneManager&) = delete;
    SceneManager& operator=(SceneManager&) = delete;

public:
    // シングルトンパターンを適用
    static SceneManager* instance;
    // インスタンスの取得
    static SceneManager* GetInstance();

    // 終了処理
    void Finalize();

    // 次シーン予約
    void SetNextScene(const std::string& sceneName);

    void Update();

    void Draw();

    const bool& EndRequest() { return roopOut_; }

private:
    // 実行中のシーン
    BaseScene* scene_ = nullptr;
    // 次のシーン
    BaseScene* nextScene_ = nullptr;

    bool roopOut_ = false;
};

