#include "BaseScene.h"
#include "SceneFactory.h"
#include <memory>

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

    void CallStart();

    const bool& EndRequest() { return loopOut_; }

private:
    bool drawStart = false;
    // 実行中のシーン
    std::unique_ptr<BaseScene> scene_ = nullptr;
    // 次のシーン
    std::unique_ptr<BaseScene> nextScene_ = nullptr;

    bool loopOut_ = false;
};

