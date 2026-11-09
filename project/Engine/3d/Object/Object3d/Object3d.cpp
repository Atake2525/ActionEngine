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

void Object3d::SetContext(Object3dContext& context) {
    m_pDirectXBase = &context.directXBase;
    m_pSrvManager = &context.srvManager;
    m_pObject3dBase = &context.object3dBase;
    m_pGameTime = &context.gameTime;
}

void Object3d::Initialize() {
    assert(m_pObject3dBase);
    Initialize(*m_pObject3dBase);
}

void Object3d::Initialize(Object3dBase& object3dBase) {

    //// Resourceの作成
    CreateTransformationMatrixResource(object3dBase.GetDirectXBase());
    //CreateLightResource();
    CreateCameraResource(object3dBase.GetDirectXBase());

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

    camera = object3dBase.GetDefaultCamera();

    cullingTemplateResource = object3dBase.GetDirectXBase().CreateBufferResource(sizeof(CullingTemplate));
    cullingTemplateResource->Map(0, nullptr, reinterpret_cast<void**>(&cullingTemplateData));

    cullingTemplateData->drawHeight = -1.0f;
    privateCullingData.drawHeight = 100.0f;

    InitializeMaterial(object3dBase.GetDirectXBase());
}

void Object3d::Update() {
    assert(m_pDirectXBase);
    assert(m_pObject3dBase);
    assert(m_pGameTime);

    auto commandList = m_pDirectXBase->GetCommandList();
    Object3dUpdateContext context{
        m_pGameTime->GetDeltaTime(),
        *commandList.Get(),
        *m_pObject3dBase,
    };
    Update(context);
}

void Object3d::Update(Object3dUpdateContext& context) {

    Matrix4x4 localMatrix = model_->GetModelData().rootNode.localMatrix;


    // 3DのTransform処理
    worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.position);
    worldMatrix = Multiply(worldMatrix, rotateQuaternionMatrix);

    if (model_->IsAnimation() && startAnimation)
    {
        // フレーム数ではなく実際の経過秒数で進め、FPSによる再生時間の差を抑える。
        animationTime += animationSpeed * context.deltaTime;
        const float duration = animation[animationKey].duration;
        // 長さ0のアニメーションではfmodによる0除算を避け、先頭姿勢を使う。
        if (duration > 0.0f)
        {
            animationTime = std::fmod(animationTime, duration);
        }
        else
        {
            animationTime = 0.0f;
        }
        if (animationTime < 0.0f) // 逆再生で負になった時刻をループ範囲に戻す。
        {
            animationTime += duration;
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
            // 補間元は切り替え時に保存した姿勢。切り替え先の再生時刻は補間中も進む。
            // startAnimationがfalseの間は補間率も進めず、その時点の姿勢を維持する。
            ChangeAnimation(animation[animationKey], animationTime, changeAnimationSpeed, context.deltaTime);
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

        Vector3 clipPos = MatrixTransform(transform.position, worldViewProjectionMatrix);

        // NDC → スクリーン座標
        /*float screenX = (clipPos.x * 0.5f + 0.5f) * (float)m_pWinApp->GetkClientWidth();
        float screenY = (1.0f - (clipPos.y * 0.5f + 0.5f)) * (float)m_pWinApp->GetkClientHeight();
        float t = screenX;*/
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
            context.commandList.SetComputeRootSignature(context.object3dBase.GetComputeRootSignature().Get());
            context.commandList.SetPipelineState(context.object3dBase.GetComputePipelineState().Get());
        }
    }

    // Culling用データの更新
    CullingTemplate data = context.object3dBase.GetCullingTemplate() + privateCullingData;
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
    assert(m_pDirectXBase);

    auto commandList = m_pDirectXBase->GetCommandList();
    Draw(*commandList.Get());
}

void Object3d::Draw(ID3D12GraphicsCommandList& commandList) {

    // wvp用のCBufferの場所を設定
    commandList.SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

    commandList.SetGraphicsRootConstantBufferView(3, cameraResource->GetGPUVirtualAddress());

    commandList.SetGraphicsRootConstantBufferView(13, cullingTemplateResource->GetGPUVirtualAddress());

    // 3Dモデルが割り当てられていれば描画する
    if (model_) {
        // wvp用のCBufferの場所を設定
        commandList.SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
        model_->Draw();
    }
}

void Object3d::CreateTransformationMatrixResource(DirectXBase& directXBase) {
    transformationMatrixResource =  directXBase.CreateBufferResource(sizeof(TransformationMatrix));
}

void Object3d::CreateCameraResource(DirectXBase& directXBase) {
    cameraResource = directXBase.CreateBufferResource(sizeof(CameraForGPU));
}

void Object3d::SetModel(Model* model, SkinClusterContext* context) {
    // モデルを検索してセットする
    model_ = model;
    // モデル変更でJointの数や順序が変わるため、以前のモデルの補間情報を破棄する。
    changingAnimation = false;
    changeAnimationTime = 0.0f;
    animationBlendStartPose.clear();
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
        SkinClusterContext localContext{ *m_pSrvManager, *m_pDirectXBase };
        SkinClusterContext& skinClusterContext = context ? *context : localContext;

        animation = model_->GetAnimation();
        skeleton = CreateSkelton(model_->GetModelData().rootNode);
        skinCluster.resize(model_->GetModelData().matVertexData.size());
        skinCluster = CreateSkinCluster(skeleton, model_->GetModelData(), skinClusterContext);
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
    // 先頭姿勢を直接適用する操作なので、残っている補間を中止する。
    // 中止しないと、次回Updateで保存済みの古い姿勢へ引き戻されてしまう。
    changingAnimation = false;
    changeAnimationTime = 0.0f;
    animationBlendStartPose.clear();
    ApplyAnimation(skeleton, animation[animationKey], animationTime);
    UpdateSkelton(skeleton);
    UpdateSkinCluster(skinCluster, skeleton);
}

void Object3d::ChangePlayAnimation(const std::string key)
{
    // 毎フレーム同じキーを要求されても、再生時刻と補間率をリセットしない。
    // 同じアニメーションを先頭から再生し直す場合はResetAnimationTimeを使う。
    if (animationKey == key)
    {
        return;
    }

    if (animation.contains(key))
    {
        // 古いアニメーションを再計算せず、直前の更新で得られた実際の姿勢をコピーする。
        // 例: Idle→Runの途中でCrouchへ変わった場合も、IdleとRunが混ざった姿勢からつなぐ。
        // 保存するのはJointのローカル姿勢で、Object3dのワールド座標ではない。
        animationBlendStartPose.clear();
        animationBlendStartPose.reserve(skeleton.joints.size());
        for (const Joint& joint : skeleton.joints)
        {
            animationBlendStartPose.push_back(joint.transform);
        }
        animationKey = key;
        // 切り替え先は先頭から再生する。ここではJointを書き換えず、次回Updateで補間する。
        animationTime = 0.0f;
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

void Object3d::AddAnimation(std::string directoryPath, std::string fileName, std::string animationName) {
    if (model_->IsAnimation())
    {
        model_->AddAnimation(directoryPath, fileName, animationName);
        animation = model_->GetAnimation();
        Log("アニメーションの読み込み完了\n");
    }
    else
    {
        Log("this object is not enable animation\n");
        Log("このオブジェクトはアニメーションが有効化されていません\n");
    }
}

void Object3d::AddAnimationsThreaded(const std::string& directoryPath, const std::vector<std::string>& fileNames) {
    if (model_->IsAnimation())
    {
        // 読み込みはModel側で並列実行し、Object3d側は反映済みのテーブルを受け取るだけにする。
        model_->AddAnimationsThreaded(directoryPath, fileNames);
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
    // 通常の補間付き切り替えとは異なり、補間を中止してDefaultAnimationを選択する。
    // 再生時刻はリセットせず、次回UpdateのApplyAnimationで姿勢を適用する。
    animationKey = "DefaultAnimation";
    changingAnimation = false;
    changeAnimationTime = 0.0f;
    animationBlendStartPose.clear();
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

void Object3d::SetTransform(const Vector3& position, const Vector3& scale, const Vector3& rotate) {
    transform.position = position;
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
            joint.transform.position = CalculateValue(rootNodeAnimation.position, animationTime);
            joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
            joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
        }
    }
}

bool Object3d::ChangeAnimation(const Animation& afterAnimation, float animationTime, float changeTime, float deltaTime)
{
    // 開始姿勢はChangePlayAnimationで、現在のSkeletonと同じJoint順に保存しておく。
    assert(animationBlendStartPose.size() == skeleton.joints.size());
    // 補間率 += 経過秒数 / 補間にかける秒数。例: 0.4秒の遷移なら0.2秒後に約0.5になる。
    // 負のdeltaTimeでは巻き戻さず、changeTimeが0以下ならその更新で補間を完了する。
    if (changeTime > 0.0f)
    {
        changeAnimationTime = std::clamp(changeAnimationTime + std::max(deltaTime, 0.0f) / changeTime, 0.0f, 1.0f);
    }
    else
    {
        changeAnimationTime = 1.0f;
    }

    float afterFrameTime = 0.0f;
    if (afterAnimation.duration > 0.0f)
    {
        afterFrameTime = std::fmod(animationTime, afterAnimation.duration);
    }
    else
    {
        afterFrameTime = 0.0f;
    }

    for (size_t index = 0; index < skeleton.joints.size(); ++index)
    {
        Joint& joint = skeleton.joints[index];
        // 開始姿勢は補間中ずっと固定する。毎フレームの補間結果を元にすると、
        // 補間率と見た目の進み方が一致しなくなるため、保存した値を使う。
        const QuaternionTransform& beforePose = animationBlendStartPose[index];
        // 切り替え先にこのJointのデータがなければ、開始時の姿勢を維持する。
        QuaternionTransform afterPose = beforePose;

        if (auto it = afterAnimation.nodeAnimations.find(joint.name); it != afterAnimation.nodeAnimations.end()) {
            const NodeAnimation& rootNodeAnimation = (*it).second;
            afterPose.position = CalculateValue(rootNodeAnimation.position, afterFrameTime);
            afterPose.rotate = CalculateValue(rootNodeAnimation.rotate, afterFrameTime);
            afterPose.scale = CalculateValue(rootNodeAnimation.scale, afterFrameTime);
        }

        // 位置とスケールは線形補間、Quaternion回転は球面線形補間でつなぐ。
        // afterPoseは再生時刻から毎回求めるので、切り替え先の動きは補間中も再生される。
        joint.transform.position = Lerp(beforePose.position, afterPose.position, changeAnimationTime);
        joint.transform.rotate = Slerp(beforePose.rotate, afterPose.rotate, changeAnimationTime);
        joint.transform.scale = Lerp(beforePose.scale, afterPose.scale, changeAnimationTime);
    }

    if (changeAnimationTime == 1.0f)
    {
        // 補間率1ではすでに切り替え先の姿勢になっているため、追加の適用は不要。
        // 保存姿勢を破棄し、次回Updateから通常のApplyAnimationへ戻す。
        changeAnimationTime = 0.0f;
        changingAnimation = false;
        animationBlendStartPose.clear();
        return true;
    }
    else
    {
        return false;
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
    capsule.start = capsulePre.start + transform.position;
    capsule.end = capsulePre.end + transform.position;
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
        joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.position);
        if (joint.parent) { // 親がいれば親の行列を掛ける
            joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skelton.joints[*joint.parent].skeletonSpaceMatrix);
        }
        else { // 親がいないのでlocalMatrixとskeltonSpaceMatrixは一致する
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }

    }
}

std::vector<SkinCluster> Object3d::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData, SkinClusterContext& context)
{
    std::vector<SkinCluster> skinCluster;
    skinCluster.resize(modelData.matVertexData.size());

    int index = 0;
    for (const auto& matVData : modelData.matVertexData)
    {
        // palette用のResourceを確保
        skinCluster[index].paletteResource = context.directXBase.CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
        WellForGPU* mappedPalette = nullptr;
        skinCluster[index].paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
        skinCluster[index].mappedPalette = { mappedPalette, skeleton.joints.size() }; // spanを使ってアクセスするようにする

        uint32_t srvIndex = context.srvManager.Allocate();

        assert(context.srvManager.CheckAllocate());

        skinCluster[index].paletteSrvHandle.first = context.srvManager.GetCPUDescriptorHandle(srvIndex);
        skinCluster[index].paletteSrvHandle.second = context.srvManager.GetGPUDescriptorHandle(srvIndex);

        // palette用のsrvを作成
        D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
        paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
        paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        paletteSrvDesc.Buffer.FirstElement = 0;
        paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        paletteSrvDesc.Buffer.NumElements = UINT(skeleton.joints.size());
        paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
        context.directXBase.GetDevice()->CreateShaderResourceView(skinCluster[index].paletteResource.Get(), &paletteSrvDesc, skinCluster[index].paletteSrvHandle.first);

        // influence用のResourceを確保
        skinCluster[index].influenceResource = context.directXBase.CreateBufferResource(sizeof(VertexInfluence) * matVData.second.vertices.size());
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

void Object3d::InitializeMaterial(DirectXBase& directXBase) {
    materialResource = directXBase.CreateBufferResource(sizeof(Material));
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
