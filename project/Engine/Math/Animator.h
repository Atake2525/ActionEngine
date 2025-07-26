
#include "kMath.h"
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <optional>
#include "Transform.h"
#include <d3d12.h>
#include <wrl.h>
#include <span>
#include <array>

#pragma once

//template <typename tValue>
//struct Keyframe
//{
//    float time;
//    tValue value;
//};
//using KeyframeVector3 = Keyframe<Vector3>;
//using KeyframeQuaternion = Keyframe<Quaternion>;

//template<typename tValue>
//struct AnimationCurve
//{
//    std::vector < Keyframe<tValue> keyframes;
//};
//
//struct NodeAnimation
//{
//    AnimationCurve<Vector3> translate;
//    AnimationCurve<Quaternion> rotate;
//    AnimationCurve<Vector3> scale;
//};


struct KeyframeVector3
{
    Vector3 value;
    float time;
};

struct KeyframeQuaternion
{
    Quaternion value;
    float time;
};

struct NodeAnimation
{
    std::vector<KeyframeVector3> translate;
    std::vector<KeyframeQuaternion> rotate;
    std::vector<KeyframeVector3> scale;
};

struct Animation
{
    float duration; // アニメーション全体の尺(単位は秒)
    // NodeAnimationの場合、Node名を弾けるようにしておく
    std::map<std::string, NodeAnimation> nodeAnimations;
};

 inline Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
    assert(!keyframes.empty()); // キーが無いのは返す値が分からないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) // キーが一つか、時刻がキーフレーム前なら最初の秒とする
    {
        return keyframes[0].value;
    }
    
    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        // indexとnextIndexの二つのkeyframeを取得して範囲内に自国があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補完する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // ここまで来た場合は一番後の時刻うよりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
}

inline Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    assert(!keyframes.empty()); // キーが無いのは返す値が分からないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) // キーが一つか、時刻がキーフレーム前なら最初の秒とする
    {
        return keyframes[0].value;
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        // indexとnextIndexの二つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補完する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t); // nextIndexとindexを逆にした。Slerpの中身同様どこかでエラーが起こるかも
        }
    }
    // ここまで来た場合は一番後の時刻うよりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
}

struct Node
{
    QuaternionTransform transform;
    Matrix4x4 localMatrix;
    std::string name;
    std::vector<Node> children;
};

struct Joint {
    QuaternionTransform transform; // Transform情報
    Matrix4x4 localMatrix; // localMatrix
    Matrix4x4 skeletonSpaceMatrix; // skeltonSpaceでの変換行列
    std::string name; // 名前
    std::vector<int32_t> children; // 子JointのIndexのリスト。居なければ空
    int32_t index; // 自分のindex
    std::optional<int32_t> parent; // 親JointのIndex。いなければnull
};

struct Skeleton {
    int32_t root; // RootJointのIndex
    std::map<std::string, int32_t> jointMap; // Joint名とIndexとの辞書
    std::vector<Joint> joints; // 所属しているジョイント
};

struct VertexWeightData
{
    float weight;
    uint32_t vertexIndex;
};

struct JointWeightData
{
    Matrix4x4 inverseBindPoseMatrix;
    std::vector<VertexWeightData> vertexWeights;
};

const uint32_t numMaxInfluence = 4;
struct VertexInfluence
{
    std::array<float, numMaxInfluence> weights;
    std::array<int32_t, numMaxInfluence> jointIndices;
};

// Paletteの色一つ一つを置く場所をWellと呼ぶ
struct WellForGPU
{
    Matrix4x4 skeletonSpaceMatrix; // 位置用
    Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
};

struct SkinCluster
{
    std::vector<Matrix4x4> inverseBindPoseMatrices;
    // Influence
    Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
    D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
    std::span<VertexInfluence> mappedInfluence;
    // MatrixPalette
    Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
    std::span<WellForGPU> mappedPalette;
    std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
};