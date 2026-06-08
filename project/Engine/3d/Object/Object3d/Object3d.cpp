#include "Object3d.h"
#include "Object3dBase.h"
#include "DirectXBase.h"
#include "TextureManager.h"
#include "Light.h"
#include "kMath.h"
#include "WinApp.h"
#include "Model.h"
#include "ModelManager.h"
#include "Collision.h"
#include "Camera.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include "SrvManager.h"
#include "Logger.h"
#include "GameTime.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

using namespace Logger;
using namespace Microsoft::WRL;

Object3d::~Object3d()
{

}

void Object3d::Initialize() {

    //// Resourceの作成
    CreateTransformationMatrixResource();
    //CreateLightResource();
    CreateCameraResource();

    // 書き込むためのアドレスを取得
    transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));
    // 書き込むためのアドレスを取得
    cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

    // 単位行列を書き込んでおく
    transformationMatrix->WVP = MakeIdentity4x4();
    transformationMatrix->World = MakeIdentity4x4();

    transform = {
        {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

    aabb = {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

    SetAxisAngle({ 0.0f, 1.0f, 0.0f });

    SetQuaternionAngle(0.0f);

    cameraData->worldPosition = { 1.0f, 1.0f, 1.0f };
    cameraData->nearClipDistance = 800.0f;
    cameraData->farClipDistance = 1000.0f;
    cameraData->drawHeihgt = 1.0f;

    camera = Object3dBase::GetInstance()->GetDefaultCamera();

    cullingTemplateResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(CullingTemplate));
    cullingTemplateResource->Map(0, nullptr, reinterpret_cast<void**>(&cullingTemplateData));

    cullingTemplateData->drawHeight = -1.0f;
    privateCullingData.drawHeight = 100.0f;

    InitializeMaterial();
}

void Object3d::Update() {

    Matrix4x4 localMatrix = model_->GetModelData().rootNode.localMatrix;


    // 3DのTransform処理
    worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
    worldMatrix = Multiply(worldMatrix, rotateQuaternionMatrix);

    if (model_->IsAnimation() && startAnimation)
    {
        animationTime += animationSpeed / std::round(1.0f / GameTime::GetInstance()->GetDeltaTime()); // 時刻を進める。1/60で固定してあるが、計測した時間を使って可変フレーム対応する方が望ましい
        animationTime = std::fmod(animationTime, animation[animationKey].duration); // 最後まで行ったら最初からリピート再生。リピートしなくても別に良い
        if (animationTime < 0.0f) // 逆再生に備えてanimationTimeが0を下回ったら最後のアニメーション時間を代入
        {
            animationTime = animation[animationKey].duration;
        }
    }
    if (model_->IsAnimation())
    {
        if (!changingAnimation)
        {
            ApplyAnimation(skeleton, animation[animationKey], animationTime);
        }
        if (changingAnimation && startAnimation)
        {
            ChangeAnimation(animation[beforAnimationKey], animation[animationKey], animationTime, changeAnimationSpeed);
        }
        UpdateSkelton(skeleton);
        UpdateSkinCluster(skinCluster, skeleton);
        model_->SkinningUpdate(skeleton);
    }

    if (isParent)
    {
        worldMatrix = Multiply(worldMatrix, parent);
    }
    else if (isTranslateParent)
    {
        Matrix4x4 translate = Multiply(worldMatrix, translateParent);
        worldMatrix.m[3][0] = translate.m[3][0];
        worldMatrix.m[3][1] = translate.m[3][1];
        worldMatrix.m[3][2] = translate.m[3][2];
    }
    else if (isRotateParent)
    {
        Matrix4x4 rotate = Multiply(worldMatrix, rotateParent);
        worldMatrix.m[0][0] = rotate.m[0][0];
        worldMatrix.m[0][1] = rotate.m[0][1];
        worldMatrix.m[0][2] = rotate.m[0][2];
        worldMatrix.m[1][0] = rotate.m[1][0];
        worldMatrix.m[1][1] = rotate.m[1][1];
        worldMatrix.m[1][2] = rotate.m[1][2];
        worldMatrix.m[2][0] = rotate.m[2][0];
        worldMatrix.m[2][1] = rotate.m[2][1];
        worldMatrix.m[2][2] = rotate.m[2][2];
    }

    Matrix4x4 worldViewProjectionMatrix;
    if (camera) {
        cameraData->worldPosition = camera->GetWorldPosition();
        cameraData->nearClipDistance = camera->GetFarClipDistance() - camera->GetFarClipDistance() * 0.2f;
        cameraData->farClipDistance = camera->GetFarClipDistance();
        cameraData->drawHeihgt = camera->GetDrawHeihgt();
        const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
        worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);

        Vector3 clipPos = MatrixTransform(transform.translate, worldViewProjectionMatrix);

        // NDC → スクリーン座標
        float screenX = (clipPos.x * 0.5f + 0.5f) * (float)WinApp::GetInstance()->GetkClientWidth();
        float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * (float)WinApp::GetInstance()->GetkClientHeight();
        float t = screenX;
    }
    else {
        worldViewProjectionMatrix = worldMatrix;
    }

    transformationMatrix->WVP = worldViewProjectionMatrix;
    transformationMatrix->World = worldMatrix;

    if (model_->IsAnimation())
    {
        transformationMatrix->WorldInverseTranspose = Inverse(worldMatrix);
        if (model_->IsAnimation())
        {
            DirectXBase::GetInstance()->GetCommandList()->SetComputeRootSignature(Object3dBase::GetInstance()->GetComputeRootSignature().Get());
            DirectXBase::GetInstance()->GetCommandList()->SetPipelineState(Object3dBase::GetInstance()->GetComputePipelineState().Get());
        }
    }

    // Culling用データの更新
    CullingTemplate data = Object3dBase::GetInstance()->GetCullingTemplate() + privateCullingData;
    cullingTemplateData->drawHeight = data.drawHeight;

    UpdateAABB();
    UpdateCapsule();

}

void Object3d::UpdateSkinCluster(std::vector<SkinCluster>& skinCluster, const Skeleton& skeleton)
{
    for (size_t skinIndex = 0; skinIndex < skinCluster.size(); skinIndex++)
    {
        for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex)
        {
            assert(jointIndex < skinCluster[skinIndex].inverseBindPoseMatrices.size());
            skinCluster[skinIndex].mappedPalette[jointIndex].skeletonSpaceMatrix = Multiply(skinCluster[skinIndex].inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
            skinCluster[skinIndex].mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = Transpose(Inverse(skinCluster[skinIndex].mappedPalette[jointIndex].skeletonSpaceMatrix));
        }
    }
}

void Object3d::Draw() {

    // wvp用のCBufferの場所を設定
    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, cameraResource->GetGPUVirtualAddress());

    DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(13, cullingTemplateResource->GetGPUVirtualAddress());

    // 3Dモデルが割り当てられていれば描画する
    if (model_) {
        // wvp用のCBufferの場所を設定
        DirectXBase::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
        model_->Draw();
    }
}

void Object3d::CreateTransformationMatrixResource() {
    transformationMatrixResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));
}

void Object3d::CreateCameraResource() {
    cameraResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(CameraForGPU));
}

void Object3d::SetModel(const std::string& filePath) {
    // モデルを検索してセットする
    model_ = ModelManager::GetInstance()->FindModel(filePath);
    first = model_->GetMeshAABB();
    aabb = first;
    auto multimeshAABBData = model_->GetMultiMeshAABB();
    multiMeshAABB.resize(model_->GetMultiMeshAABB().size());
    for (const auto data : multimeshAABBData)
    {
        firstMultiMeshAABB.push_back(data.second);
    }
    CreateCapsule();
    if (model_->IsAnimation())
    {
        animation = model_->GetAnimation();
        skeleton = CreateSkelton(model_->GetModelData().rootNode);
        skinCluster.resize(model_->GetModelData().matVertexData.size());
        skinCluster = CreateSkinCluster(skeleton, model_->GetModelData());
        model_->SetSkinCluster(skinCluster);
        // GPUskinning用リソースはModel側でメッシュ数に合わせて確保する。
        model_->CreateSkinningResources(skeleton);
        ApplyAnimation(skeleton, animation[animationKey], animationTime);
        UpdateSkelton(skeleton);
        UpdateSkinCluster(skinCluster, skeleton);
    }
}

void Object3d::SetModel(const std::string& directoryPath, const std::string& filePath, const bool& enableLighting, const bool isAnimation) {
    ModelManager::GetInstance()->LoadModel(directoryPath, filePath, isAnimation);

    // ディレクトリの最後の名前もモデルのkeyに入れる
    size_t pathLen = directoryPath.size();
    size_t pathNum = 0;
    for (size_t i = directoryPath.size(); i > 0; --i)
    {
        char c = directoryPath[i - 1];
        if (c == '/') {
            pathNum = i;
            pathLen = directoryPath.size() - i;
            break;
        }
    }
    std::string filename;
    for (size_t i = 0; i < pathLen; i++)
    {
        char c = directoryPath[pathNum + i];
        filename += c;
    }

    filename = filename + '/' + filePath;

    // モデルを検索してセットする
    model_ = ModelManager::GetInstance()->FindModel(filename);
    first = model_->GetMeshAABB();
    aabb = first;
    auto multimeshAABBData = model_->GetMultiMeshAABB();
    multiMeshAABB.resize(model_->GetMultiMeshAABB().size());
    for (const auto data : multimeshAABBData)
    {
        firstMultiMeshAABB.push_back(data.second);
    }
    CreateCapsule();
    if (model_->IsAnimation())
    {
        animation = model_->GetAnimation();
        skeleton = CreateSkelton(model_->GetModelData().rootNode);
        skinCluster.resize(model_->GetModelData().matVertexData.size());
        skinCluster = CreateSkinCluster(skeleton, model_->GetModelData());
        model_->SetSkinCluster(skinCluster);
        // GPUskinning用リソースはModel側でメッシュ数に合わせて確保する。
        model_->CreateSkinningResources(skeleton);
        ApplyAnimation(skeleton, animation[animationKey], animationTime);
        UpdateSkelton(skeleton);
        UpdateSkinCluster(skinCluster, skeleton);
    }
}

void Object3d::ResetAnimationTime()
{
    animationTime = 0.0f;
    ApplyAnimation(skeleton, animation[animationKey], animationTime);
    UpdateSkelton(skeleton);
    UpdateSkinCluster(skinCluster, skeleton);
}

void Object3d::ChangePlayAnimation(const std::string key)
{
    if (animation.contains(key))
    {
        beforAnimationKey = animationKey;
        animationKey = key;
        changingAnimation = true;
        changeAnimationTime = 0.0f;
        //Log("再生されるアニメーションが変更されました\n");
    }
    else
    {
        Log("this key is not loaded now\n");
        Log("指定のキーのアニメーションは追加されていません\n");
    }

}

void Object3d::AddAnimation(std::string directoryPath, std::string filename, std::string animationName) {
    if (model_->IsAnimation())
    {
        model_->AddAnimation(directoryPath, filename, animationName);
        animation = model_->GetAnimation();
        Log("アニメーションの読み込み完了\n");
    }
    else
    {
        Log("this object is not enable animation\n");
        Log("このオブジェクトはアニメーションが有効化されていません\n");
    }
}

void Object3d::AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& filenames) {
    if (model_->IsAnimation())
    {
        // 読み込みはModel側で並列実行し、Object3d側は反映済みのテーブルを受け取るだけにする。
        model_->AddAnimationsThreaded(directoryPath, filenames);
        animation = model_->GetAnimation();
        Log("アニメーションの並列読み込み完了\n");
    }
    else
    {
        Log("this object is not enable animation\n");
        Log("このオブジェクトはアニメーションが有効化されていません\n");
    }
}

void Object3d::PlayDefaultAnimation() {
    beforAnimationKey = animationKey;
    animationKey = "DefaultAnimation";
    changingAnimation = false;
    Log("initialized animation");
    Log("アニメーションを初期化しました\n");
    if (!model_->IsAnimation())
    {
        Log("this object is not enable animation");
        Log("このオブジェクトはアニメーションが有効化されていません\n");
    }
}

void Object3d::SetColor(const Vector4 color) {
    materialData->color = color;
}

const Vector4& Object3d::GetColor() const {
    return materialData->color;
}

const bool Object3d::GetEnableLighting() const {
    return materialData->enableLighting;
}

void Object3d::SetEnableLighting(const bool enableLighting) {
    materialData->enableLighting = enableLighting;
}

const Vector3 Object3d::GetRotateInDegree() const {
    return SwapDegree(transform.rotate);
}

void Object3d::SetRotateInDegree(const Vector3& rotate) {
    transform.rotate = SwapRadian(rotate);
}

void Object3d::SetTransform(const Vector3& translate, const Vector3& scale, const Vector3& rotate) {
    transform.translate = translate;
    transform.scale = scale;
    transform.rotate = rotate;
}

const float& Object3d::GetShininess() const {
    return materialData->shininess;
}

void Object3d::SetShininess(const float& shininess) {
    materialData->shininess = shininess;
}

void Object3d::CreateCapsule() {
    Vector3 center = CenterAABB(first);
    float dist = Distance(center, Vector3{ center.x, center.y, first.min.z });
    capsulePre.start = { center.x, first.min.y + dist, center.z };
    capsulePre.end = { center.x, first.max.y - dist, center.z };

    capsulePre.radius = dist;
    capsule = capsulePre;

}

void Object3d::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
    for (Joint& joint : skeleton.joints)
    {
        // 対象のJointのAnimationがあれば、値の適用を行う。 下記のif分はc++17から可能になった初期化付きif文
        if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
            const NodeAnimation& rootNodeAnimation = (*it).second;
            joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
            joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
            joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
        }
    }
}

const bool Object3d::ChangeAnimation(Animation& beforAnimation, Animation& afterAnimation, float animationTime, float changeTime)
{
    changeAnimationTime += 1.0f / 60.0f / changeTime;
    float beforFrameTime = animationTime;
    beforFrameTime = std::fmod(beforFrameTime, beforAnimation.duration);
    float afterFrameTime = animationTime;
    afterFrameTime = std::fmod(afterFrameTime, afterAnimation.duration);
    for (Joint& joint : skeleton.joints)
    {
        Vector3 beforTranslate, afterTranslate;
        Quaternion beforRotate, afterRotate;
        Vector3 beforScale, afterScale;
        beforTranslate = joint.transform.translate;
        beforRotate = joint.transform.rotate;
        beforScale = joint.transform.scale;
        bool beforSuccess = false, afterSuccess = false;
        // 対象のJointのAnimationがあれば、値の適用を行う。 下記のif分はc++17から可能になった初期化付きif文
        if (auto it = beforAnimation.nodeAnimations.find(joint.name); it != beforAnimation.nodeAnimations.end()) {
            const NodeAnimation rootNodeAnimation = (*it).second;
            beforTranslate = CalculateValue(rootNodeAnimation.translate, beforFrameTime);
            beforRotate = CalculateValue(rootNodeAnimation.rotate, beforFrameTime);
            beforScale = CalculateValue(rootNodeAnimation.scale, beforFrameTime);
            beforSuccess = true;
        }

        // 対象のJointのAnimationがあれば、値の適用を行う。 下記のif分はc++17から可能になった初期化付きif文
        if (auto it = afterAnimation.nodeAnimations.find(joint.name); it != afterAnimation.nodeAnimations.end()) {
            const NodeAnimation rootNodeAnimation = (*it).second;
            afterTranslate = CalculateValue(rootNodeAnimation.translate, afterFrameTime);
            afterRotate = CalculateValue(rootNodeAnimation.rotate, afterFrameTime);
            afterScale = CalculateValue(rootNodeAnimation.scale, afterFrameTime);
            afterSuccess = true;
        }

        changeAnimationTime = std::clamp(changeAnimationTime, 0.0f, 1.0f);

        if (beforSuccess && afterSuccess)
        {
            // 両方の処理ができていれば実行する
            joint.transform.translate = Lerp(beforTranslate, afterTranslate, changeAnimationTime);
            joint.transform.rotate = Slerp(beforRotate, afterRotate, changeAnimationTime);
            joint.transform.scale = Lerp(beforScale, afterScale, changeAnimationTime);
        }
        if (changeAnimationTime == 1.0f)
        {
            for (Joint& joint : skeleton.joints)
            {
                // 対象のJointのAnimationがあれば、値の適用を行う。 下記のif分はc++17から可能になった初期化付きif文
                if (auto it = afterAnimation.nodeAnimations.find(joint.name); it != afterAnimation.nodeAnimations.end()) {
                    const NodeAnimation& rootNodeAnimation = (*it).second;
                    joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
                    joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
                    joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
                }
            }
            this->animationTime = afterFrameTime;
        }
    }

    if (changeAnimationTime == 1.0f)
    {
        changeAnimationTime = 0.0f;
        changingAnimation = false;
        return true;
    }
    else
    {
        return false;
    }
}

void Object3d::DebugMode(const bool debugMode) {
    if (model_)
    {
        model_->DebugMode(debugMode);
    }
}

const Vector3 Object3d::GetJointPosition(const std::string jointName)
{
    size_t nameSize = jointName.size();
    size_t hitSize = nameSize;
    for (const Joint& joint : skeleton.joints)
    {
        for (size_t i = 0; i < joint.name.size(); i++)
        {
            char c = joint.name[i];
            if (c == jointName[0 + hitSize])
            {
                ++hitSize;
            }
            else
            {
                hitSize = 0;
            }
            if (hitSize == nameSize)
            {
                break;
            }
        }
        if (hitSize == nameSize)
        {
            //Log("目標が見つかりました\n");
            Matrix4x4 jointMatrix = Multiply(joint.skeletonSpaceMatrix, worldMatrix);
            return { jointMatrix.m[3][0], jointMatrix.m[3][1], jointMatrix.m[3][2] };
        }
    }
    Log("目標が見つかりませんでした\n");
    return { 0.0f, 0.0f, 0.0f };
}

const Vector3 Object3d::GetJointNormal(const std::string jointName)
{
    size_t nameSize = jointName.size();
    size_t hitSize = nameSize;
    for (const Joint& joint : skeleton.joints)
    {
        for (size_t i = 0; i < joint.name.size(); i++)
        {
            char c = joint.name[i];
            if (c == jointName[0 + hitSize])
            {
                ++hitSize;
            }
            else
            {
                hitSize = 0;
            }
            if (hitSize == nameSize)
            {
                break;
            }
        }
        if (hitSize == nameSize)
        {
            Log("目標が見つかりました\n");
            Matrix4x4 jointMatrix = Multiply(joint.skeletonSpaceMatrix, worldMatrix);
            return { jointMatrix.m[0][2], jointMatrix.m[1][2], jointMatrix.m[2][2] };
        }
    }
    Log("目標が見つかりませんでした\n");
    return { 0.0f, 0.0f, 0.0f };
}

const Matrix4x4 Object3d::GetJointMatrix(const std::string& jointName)
{
    size_t nameSize = jointName.size();
    size_t hitSize = nameSize;
    for (const Joint& joint : skeleton.joints)
    {
        for (size_t i = 0; i < joint.name.size(); i++)
        {
            char c = joint.name[i];
            if (c == jointName[0 + hitSize])
            {
                ++hitSize;
            }
            else
            {
                hitSize = 0;
            }
            if (hitSize == nameSize)
            {
                break;
            }
        }
        if (hitSize == nameSize)
        {
            Log("目標が見つかりました\n");
            Matrix4x4 jointMatrix = Multiply(joint.skeletonSpaceMatrix, worldMatrix);
            return jointMatrix;
        }
    }
    Log("目標が見つかりませんでした\n");
    return { 0.0f, 0.0f, 0.0f };
}

const bool Object3d::CheckCollisionAABB(Object3d* object) const {
    return CollisionAABB(aabb, object->GetAABB());
}

void Object3d::UpdateOBB()
{
    Vector3 halfSize = { (aabb.max.x - aabb.min.x) / 2.0f, (aabb.max.y - aabb.min.y) / 2.0f, (aabb.max.z - aabb.min.z) / 2.0f };

    obb = CreateOBB(worldMatrix, halfSize);

    multiMeshOBB.clear();

    for (size_t index = 0; index < multiMeshAABB.size(); index++)
    {
        Vector3 multiHalfSize = { (multiMeshAABB[index].max.x - multiMeshAABB[index].min.x) / 2.0f, (multiMeshAABB[index].max.y - multiMeshAABB[index].min.y) / 2.0f, (multiMeshAABB[index].max.z - multiMeshAABB[index].min.z) / 2.0f };

        multiMeshOBB.push_back(CreateOBB(worldMatrix, multiHalfSize, CenterAABB(multiMeshAABB[index])));
    }
}

const std::vector<AABB>& Object3d::GetAABBMultiMeshed()
{
    return multiMeshAABB;
}

const bool Object3d::CheckCollisionAABBs(Object3d* object) const
{
    for (AABB objectAABB : object->GetAABBMultiMeshed())
    {
        if (CollisionAABB(aabb, objectAABB))
        {
            return true;
        }
    }
    return false;
}

const bool Object3d::CheckCollisionCapsule(Object3d* object) const
{
    return CollisionCapsuleAABB(capsule, object->GetAABB());
}

const bool Object3d::CheckCollisionOBB(Object3d* object) const
{
    return CheckOBBCollision(obb, object->GetOBB());
}

const bool Object3d::CheckCollisionOBBs(Object3d* object) const
{
    for (auto colOBB : multiMeshOBB)
    {
        if (CheckOBBCollision(colOBB, object->GetOBB()))
        {
            return true;
        }
    }
    return false;
}

const bool Object3d::CheckCollisionOBB(const OBB& obb) const
{
    return CheckOBBCollision(this->obb, obb);
}

const bool Object3d::CheckCollisionOBBs(const OBB& obb) const
{
    for (auto colOBB : multiMeshOBB)
    {
        if (CheckOBBCollision(colOBB, obb))
        {
            return true;
        }
    }
    return false;
}

void Object3d::UpdateAABB()
{
    aabbPre = aabb;
    multiMeshAABBPre = multiMeshAABB;

    Vector3 worldPos = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };


    aabb.min = (first.min * transform.scale) + worldPos;
    aabb.max = (first.max * transform.scale) + worldPos;

    for (size_t index = 0; index < multiMeshAABB.size(); index++)
    {
        multiMeshAABB[index].min = (firstMultiMeshAABB[index].min * transform.scale) + worldPos;
        multiMeshAABB[index].max = (firstMultiMeshAABB[index].max * transform.scale) + worldPos;
    }

}

void Object3d::UpdateCapsule()
{
    // カプセルの更新
    capsule.start = capsulePre.start + transform.translate;
    capsule.end = capsulePre.end + transform.translate;
}

const Skeleton Object3d::CreateSkelton(const Node& rootNode)
{
    Skeleton skeleton;
    skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

    // 名前とindexのマッピングを行いアクセスしやすくする
    for (const Joint& joint : skeleton.joints)
    {
        skeleton.jointMap.emplace(joint.name, joint.index);
    }
    UpdateSkelton(skeleton);
    return skeleton;
}

const int32_t Object3d::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints)
{
    Joint joint;
    joint.name = node.name;
    joint.localMatrix = node.localMatrix;
    joint.skeletonSpaceMatrix = MakeIdentity4x4();
    joint.transform = node.transform;
    if (node.transform.rotate.w > 1.0f || node.transform.rotate.w < -1.0f)
    {
        assert(false);
    }
    joint.index = int32_t(joints.size()); // 現在登録されている数をIndexに
    joint.parent = parent;
    joints.push_back(joint); // SkeltonのJoint列に追加
    for (const Node& child : node.children) {
        // 子Jointを作成し、そのIndexを登録
        int32_t childIndex = CreateJoint(child, joint.index, joints);
        joints[joint.index].children.push_back(childIndex);
    }
    // 自身のindexを返す
    return joint.index;

}

void Object3d::UpdateSkelton(Skeleton& skelton)
{
    // 全てのJointを更新。親が若いので通常ループで処理可能になっている
    for (Joint& joint : skelton.joints)
    {
        joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
        if (joint.parent) { // 親がいれば親の行列を掛ける
            joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skelton.joints[*joint.parent].skeletonSpaceMatrix);
        }
        else { // 親がいないのでlocalMatrixとskeltonSpaceMatrixは一致する
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }

    }
}

std::vector<SkinCluster> Object3d::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData)
{
    std::vector<SkinCluster> skinCluster;
    skinCluster.resize(modelData.matVertexData.size());

    int index = 0;
    for (const auto& matVData : modelData.matVertexData)
    {
        // palette用のResourceを確保
        skinCluster[index].paletteResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
        WellForGPU* mappedPalette = nullptr;
        skinCluster[index].paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
        skinCluster[index].mappedPalette = { mappedPalette, skeleton.joints.size() }; // spanを使ってアクセスするようにする

        uint32_t srvIndex = SrvManager::GetInstance()->Allocate();

        assert(SrvManager::GetInstance()->CheckAllocate());

        skinCluster[index].paletteSrvHandle.first = SrvManager::GetInstance()->GetCPUDescriptorHandle(srvIndex);
        skinCluster[index].paletteSrvHandle.second = SrvManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);

        // palette用のsrvを作成
        D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
        paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
        paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        paletteSrvDesc.Buffer.FirstElement = 0;
        paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
        paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
        DirectXBase::GetInstance()->GetDevice()->CreateShaderResourceView(skinCluster[index].paletteResource.Get(), &paletteSrvDesc, skinCluster[index].paletteSrvHandle.first);

        // influence用のResourceを確保
        skinCluster[index].influenceResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(VertexInfluence) * matVData.second.vertices.size());
        VertexInfluence* mappedInfluence = nullptr;
        skinCluster[index].influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
        std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * matVData.second.vertices.size()); // 0埋め。weightを0にしておく
        skinCluster[index].mappedInfluence = { mappedInfluence, matVData.second.vertices.size() };

        // influence用のVBVを作成
        skinCluster[index].influenceBufferView.BufferLocation = skinCluster[index].influenceResource->GetGPUVirtualAddress();
        skinCluster[index].influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * matVData.second.vertices.size());
        skinCluster[index].influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

        // InverseBindPoseMatrixの保存領域を作成
        skinCluster[index].inverseBindPoseMatrices.resize(skeleton.joints.size());
        std::generate(skinCluster[index].inverseBindPoseMatrices.begin(), skinCluster[index].inverseBindPoseMatrices.end(), MakeIdentity4x4);
        // ModelDataのSkinCluster情報を解析してInfluenceの中身を埋める
        for (const auto& jointWeight : matVData.second.skinClusterData) // ModelのSkinClusterの方法を解析
        {
            auto it = skeleton.jointMap.find(jointWeight.first); // jointWeight.firstはjoint名なので、skeletonに対象となるJointが含まれているか判断
            if (it == skeleton.jointMap.end()) // そんな名前のJointは存在しない。なので次に回す
            {
                continue;
            }
            // (*it).secondにはjointのindexが入っているので、外套のindexのinverseBindPoseMatrixを代入
            skinCluster[index].inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
            for (const auto& vertexWeight : jointWeight.second.vertexWeights)
            {
                auto& currentInfluence = skinCluster[index].mappedInfluence[vertexWeight.vertexIndex]; // 外套のvertexIndexのinfluence情報を参照しておく
                for (uint32_t i = 0; i < numMaxInfluence; ++i) // 空いているところに入れる
                {
                    if (currentInfluence.weights[i] == 0.0f) // weight==0が空いている状態なので、その場所にweightとjointのindexを代入
                    {
                        currentInfluence.weights[i] = vertexWeight.weight;
                        currentInfluence.jointIndices[i] = (*it).second;
                        skinCluster[index].mappedInfluence[vertexWeight.vertexIndex] = currentInfluence;
                        break;
                    }
                }
            }
        }
        index++;
    }

    return skinCluster;
}

const float Object3d::GetEnvironmentCoefficient() const {
    return materialData->environmentCoefficient;
}

void Object3d::SetEnvironmentCoefficient(const float amount) {
    materialData->environmentCoefficient = amount;
}

void Object3d::SetPBRMaterial(const float metallic, const float roughness) {
    model_->SetPBRMaterial(metallic, roughness);
}

void Object3d::InitializeMaterial() {
    materialResource = DirectXBase::GetInstance()->CreateBufferResource(sizeof(Material));
    //  書き込むためのアドレスを取得
    materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

    // データを書き込む
    // 今回は赤を書き込んでみる
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };

    materialData->uvTransform = MakeIdentity4x4();

    materialData->enableLighting = false;
    materialData->shininess = 70.0f;
    materialData->specularColor = { 1.0f, 1.0f, 1.0f };
    materialData->environmentCoefficient = 0.0f;
    materialData->enableMetallic = false;
}