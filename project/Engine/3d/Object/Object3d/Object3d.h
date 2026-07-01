#pragma once

// ============================
//  Include
// ============================
#include <vector>
#include <map>
#include <string>
#include <wrl.h>
#include <d3d12.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Quaternion.h"
#include "kMath.h"

#include "Animator.h"
#include "Model.h"

#include "AABB.h"
#include "OBB.h"
#include "Capsule.h"

#include "Culling.h"

class Camera;
class Object3dBase;
class DirectXBase;
class WinApp;
class SrvManager;
class GameTime;

struct Object3dUpdateContext {
    float deltaTime;
    ID3D12GraphicsCommandList& commandList;
    Object3dBase& object3dBase;
};

struct SkinClusterContext {
    SrvManager& srvManager;
    DirectXBase& directXBase;
};

struct Object3dContext {
    DirectXBase& directXBase;
    SrvManager& srvManager;
    Object3dBase& object3dBase;
    GameTime& gameTime;
};

// ============================
//  Object3d
// ============================
class Object3d {
public:
    // ============================
    //  Lifecycle
    // ============================
    void SetContext(Object3dContext& context);
    void Initialize();
    void Initialize(Object3dBase& object3dBase);
    void Update();
    void Update(Object3dUpdateContext& context);
    void Draw();
    void Draw(ID3D12GraphicsCommandList& commandList);
    ~Object3d();

    // ============================
    //  Model
    // ============================
    void SetModel(Model* model, SkinClusterContext* context = nullptr);
    // ============================
    //  Camera
    // ============================
    void SetCamera(Camera* camera) { this->camera = camera; }

    // ============================
    //  Animation
    // ============================
    void ToggleStartAnimation() { startAnimation = !startAnimation; }
    void SetStartAnimation(bool start) { startAnimation = start; }
    void ResetAnimationTime();
    void AddAnimation(std::string directoryPath, std::string fileName, std::string animationName);
    void AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& fileNames);
    void PlayDefaultAnimation();
    void ChangePlayAnimation(const std::string key = "DefaultAnimation");
    const std::string& GetCurrentAnimationKey() const { return animationKey; }
    void ResetAnimationSpeed() { animationSpeed = 1.0f; }
    const float& GetChangeAnimationSpeed() const { return changeAnimationSpeed; }
    void SetChangeAnimationSpeed(const float speed = 0.4f) { changeAnimationSpeed = speed; }
    void SetAnimationSpeed(const float speed) { animationSpeed = speed; }
    const float& GetAnimationSpeed() const { return animationSpeed; }

    // ============================
    //  Transform
    // ============================
    const Transform& GetTransform() const { return transform; }
    const Vector3& GetPosition() const { return transform.position; }
    const Vector3& GetScale() const { return transform.scale; }
    const Vector3& GetRotate() const { return transform.rotate; }
    const Vector3 GetRotateInDegree() const;

    void SetTransform(const Transform& transform) { this->transform = transform; }
    void SetTransform(const Vector3& position, const Vector3& scale, const Vector3& rotate);
    void SetPosition(const Vector3& position) { transform.position = position; }
    void SetScale(const Vector3& scale) { transform.scale = scale; }
    void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
    void SetRotateInDegree(const Vector3& rotate);

    // 任意軸回転
    void SetAxisAngle(const Vector3& rotate) { axisAngle = Normalize(rotate); }
    void SetQuaternionAngle(const float& angle) { rotateQuaternionMatrix = MakeRotateAxisAngle(axisAngle, angle); }

    // ============================
    //  Parent Transform
    // ============================
    void SetParent(const Matrix4x4& worldMatrix) {
        parent = worldMatrix;
        isParent = true;
        isTranslateParent = false;
        isRotateParent = false;
    }
    void DeleteParent() { isParent = false; }

    void SetTranslateParent(const Matrix4x4& worldMatrix) {
        translateParent = worldMatrix;
        isTranslateParent = true;
        isParent = false;
        isRotateParent = false;
    }
    void DeleteTranslateParent() { isTranslateParent = false; }

    void SetRotateParent(const Matrix4x4& worldMatrix) {
        rotateParent = worldMatrix;
        isRotateParent = true;
        isTranslateParent = false;
        isParent = false;
    }
    void DeleteRotateParent() { isRotateParent = false; }

    // ============================
    //  Rendering
    // ============================
    const Vector4& GetColor() const;
    void SetColor(const Vector4 color);
    void SetEnableLighting(const bool enableLighting);
    const bool GetEnableLighting() const;
    const float GetEnvironmentCoefficient() const;
    void SetEnvironmentCoefficient(const float amount);
    void SetShininess(const float& shininess);
    const float& GetShininess() const;
    void SetEnableMetallic(const bool flag) { materialData->enableMetallic = flag; }
    const bool GetEnableMetallic() { return materialData->enableMetallic; }
    void SetPBRMaterial(const float metallic, const float roughness);

    // ============================
    //  Culling
    // ============================
    void SetDrawHeiht(const float height) { privateCullingData.drawHeight = height; }
    const CullingTemplate GetCullingTemplateData() const { return privateCullingData; }

    // ============================
    //  Collision
    // ============================
    const AABB& GetAABB() const { return aabb; }
    const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
    const Capsule& GetCapsule() const { return capsule; }
    const OBB& GetOBB() const { return obb; }    const std::vector<OBB>& GetMultiMeshOBB() const { return multiMeshOBB; }

    const bool CheckCollisionAABB(Object3d* object) const;
    const bool CheckCollisionAABBs(Object3d* object) const;
    const bool CheckCollisionCapsule(Object3d* object) const;
    const bool CheckCollisionOBB(Object3d* object) const;
    const bool CheckCollisionOBBs(Object3d* object) const;
    const bool CheckCollisionOBB(const OBB& obb) const;
    const bool CheckCollisionOBBs(const OBB& obb) const;

    void UpdateAABB();
    void UpdateCapsule();
    void UpdateOBB();;

    const std::vector<AABB>& GetAABBMultiMeshed();
    void CreateCapsule();

    // ============================
    //  Joint Info
    // ============================
    const Vector3 GetJointPosition(const std::string jointName);
    const Vector3 GetJointNormal(const std::string jointName);
    const Matrix4x4 GetJointMatrix(const std::string& jointName);

private:
    DirectXBase* m_pDirectXBase = nullptr;
    SrvManager* m_pSrvManager = nullptr;
    Object3dBase* m_pObject3dBase = nullptr;
    GameTime* m_pGameTime = nullptr;

    // ============================
    //  Transform Data
    // ============================
    Transform transform;
    Vector3 axisAngle;
    Matrix4x4 rotateQuaternionMatrix;

    Matrix4x4 parent;
    bool isParent = false;
    Matrix4x4 translateParent;
    bool isTranslateParent = false;
    Matrix4x4 rotateParent;
    bool isRotateParent = false;

    Matrix4x4 worldMatrix;

    // ============================
    //  Camera Data
    // ============================
    Camera* camera = nullptr;

    struct CameraForGPU {
        Vector3 worldPosition;
        float nearClipDistance;
        float farClipDistance;
        float drawHeihgt;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
    CameraForGPU* cameraData = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    Material* materialData = nullptr;

    // ============================
    //  Rendering Data
    // ============================
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
    TransformationMatrix* transformationMatrix = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> cullingTemplateResource;
    CullingTemplate* cullingTemplateData = nullptr;
    CullingTemplate privateCullingData;

    // ============================
    //  Model Data
    // ============================
    Model* model_ = nullptr;

    // ============================
    //  Animation Data
    // ============================
    std::unordered_map<std::string, Animation> animation;
    std::string beforAnimationKey = "DefaultAnimation";
    std::string animationKey = "DefaultAnimation";

    bool changingAnimation = false;
    float animationTime = 0.0f;
    float changeAnimationTime = 0.0f;
    float changeAnimationSpeed = 0.4f;

    bool startAnimation = false;
    float animationSpeed = 1.0f;

    Skeleton skeleton;
    std::vector<SkinCluster> skinCluster;


    // ============================
    //  Collision Data
    // ============================
    AABB aabb;
    AABB aabbPre;
    std::vector<AABB> multiMeshAABB;
    std::vector<AABB> multiMeshAABBPre;

    AABB first;
    std::vector<AABB> firstMultiMeshAABB;

    Capsule capsule;
    Capsule capsulePre;

    OBB obb;
    std::vector<OBB> multiMeshOBB;

private:

    // MaterialResourceを作成する
    void InitializeMaterial(DirectXBase& directXBase);

    // ============================
    //  Internal Methods
    // ============================
    void CreateTransformationMatrixResource(DirectXBase& directXBase);
    void CreateCameraResource(DirectXBase& directXBase);

    void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);
    const bool ChangeAnimation(Animation& beforAnimation, Animation& afterAnimation,
        float animationTime, float changeTime);

    const Skeleton CreateSkelton(const Node& rootNode);
    const int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent,
        std::vector<Joint>& joints);

    void UpdateSkelton(Skeleton& skelton);
    std::vector<SkinCluster> CreateSkinCluster(const Skeleton& skeleton,
        const ModelData& modelData, SkinClusterContext& context);

    void UpdateSkinCluster(std::vector<SkinCluster>& skinCluster, const Skeleton& skeleton);
};
