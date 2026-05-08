#include "Object3d.h"
#include "Object3dBase.h"
#include "SpriteBase.h"
#include "Camera.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Input.h"
#include "WireFrameObjectBase.h"
#include "SkinningObject3dBase.h"
#include "Sprite.h"
#include "AABB.h"
#include "ParticleManager.h"
#include "Audio.h"
#include "BaseScene.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include <map>
#include "UI.h"
#include <memory>
#include "MouseCursor.h"
#include "TitleSceneUI.h"

#pragma once

// タイトルシーン
class TitleScene : public BaseScene
{
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    const bool& EndRequest() override { return finished; }

private:
    bool finished = false;

    TitleSceneScreen m_sceneScreen = TitleSceneScreen::BootScreen;
    enum class Select {
        Play = 0,
        Setting = 1,
        Exit = 2,
        Credit = 3,
    };

    bool m_screenChange = false;
    float m_screenChangeTimer = 0.0f;
    float m_screenChangeTime[2] = {1.3f, 1.0f };
    int m_changeNum = 0;

    Transform m_screenChangeTransformPre = Transform::Default;
    Transform m_screenChangeTransform[2] = {
        {
            {1.0f, 1.0f, 1.0f},
            {SwapRadian(11.5f), SwapRadian(1.5f), 0.0f},
            {0.0f, 2.1f, 1.6f},
        },
        {
            {1.0f, 1.0f, 1.0f},
            {SwapRadian(11.5f), SwapRadian(1.5f), 0.0f},
            {0.0f, 2.1f, 3.1f},
        }
    };

    std::unique_ptr<Camera> m_pCamera;
    Input* m_pInput = nullptr;

    // BootScreen

    std::unique_ptr<Object3d> m_bootScreen = nullptr;

    std::unique_ptr<Object3d> m_charModel = nullptr;


    // TitleScreen 

    std::unique_ptr<TitleSceneUI> m_titleSceneUI = nullptr;

    int m_maxSelectNum = 3;

    Select m_select = Select::Play;
    Select m_selectPre = Select::Play;

    std::unique_ptr<Sprite> m_gamePad = nullptr;

    std::unique_ptr<Sprite> m_credit_sound = nullptr;
    std::unique_ptr<Sprite> m_credit = nullptr;
    bool m_showCredit = false;
    bool m_start = false;

    std::unique_ptr<MouseCursor> m_mouseCursor;

};

