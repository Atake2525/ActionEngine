#include "Object3d.h"

class Camera;

class Player {
private:
    enum class PlayerState {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling
    };
public:
    Player();
    ~Player();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="camera">使用しているカメラ</param>
    /// <param name="jsonName"></param>
    /// <param name="DebugMode"></param>
    void Initialize(Camera* camera, std::string jsonName, const bool DebugMode = false);

};