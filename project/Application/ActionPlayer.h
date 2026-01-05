#include "Transform.h"
#include "Object3d.h"
#include <memory>

class Camera;

class ActionPlayer {
private:
    
public:
    ActionPlayer();
    ~ActionPlayer();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(Camera* camera);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 処理を凍結
    /// </summary>
    void SetFreeze(bool freeze) { m_freeze = freeze; }

private:
    Camera* m_pCamera;
    std::unique_ptr<Object3d> playerObj;

    bool m_freeze = false;

    Vector3 m_moveVelocity = Vector3::Zero;

    float m_walkSpeed = 1.2f;

};