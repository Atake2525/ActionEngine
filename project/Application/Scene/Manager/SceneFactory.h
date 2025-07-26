#pragma once

#include "AbstractSceneFactory.h"

class SceneFactory : public AbstractSceneFactory
{
private:

    // コンストラクタ、デストラクタの隠蔽
    SceneFactory() = default;
    ~SceneFactory() = default;
    // コピーコンストラクタ、コピー代入演算子の封印
    SceneFactory(SceneFactory&) = delete;
    SceneFactory& operator=(SceneFactory&) = delete;


public:

    // シングルトンパターンを適用
    static SceneFactory* instance;
    // インスタンスの取得
    static SceneFactory* GetInstance();

    // 終了処理
    void Finalize();

    /// <summary>
    /// シーン生成
    /// </summary>
    /// <param name="sceneName">シーン名</param>
    /// <returns></returns>
    BaseScene* ChangeScene(const std::string& sceneName) override;
};

