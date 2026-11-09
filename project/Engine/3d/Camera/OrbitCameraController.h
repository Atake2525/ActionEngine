#include "Camera.h"
#pragma once

class OrbitCameraController {
public:
    struct OrbitCameraParam
    {
        Vector3 target; // ワールド座標
        float yaw; // ラジアン
        float pitch; // ラジアン
        float distance;
    };

    explicit OrbitCameraController(Camera& camera);
    ~OrbitCameraController();

    void Update(const OrbitCameraParam& param);
private:
    Camera* m_camera = nullptr;
};