#include "OrbitCameraController.h"
#include "kMath.h"

OrbitCameraController::OrbitCameraController(Camera& camera) {
    m_camera = &camera;
}

OrbitCameraController::~OrbitCameraController(){}

void OrbitCameraController::Update(const OrbitCameraParam& param) {
    Vector3 orbitCameraPos = CalculateOrbitPosition(param.target, param.distance, param.yaw, param.pitch);
    m_camera->SetPosition(orbitCameraPos);
    m_camera->SetRotate({ param.pitch, param.yaw, 0.0f });
}