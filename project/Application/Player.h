#include "Object3d.h"
#include <memory>
#pragma once
#include <string>

class Camera;

class Player
{
private:
    enum class PlayerState {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling
    };
    PlayerState currentState = PlayerState::Idle;
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">現在使用しているカメラ</param>
    /// <param name="jsonName">ステージのJsonData</param>
    void Initialize(Camera* camera, const std::string& jsonName);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();


private:

    std::unique_ptr<Object3d> m_pModel;
    Camera* m_pCamera = nullptr;

    // プレイヤmeaningーの移動
    Transform m_transform = Transform::Default;
    Transform m_velocity = Transform::Default;

    // カメラ関連
    Transform m_cameraTransform = Transform::Default;
    Transform m_cameraVelocity = Transform::Default;
    float m_fovY = 60.0f;
    float m_afterFovY = 60.0f;
    float m_fovTimer = 0.0f;
    float m_fovTime = 0.3f;

};

