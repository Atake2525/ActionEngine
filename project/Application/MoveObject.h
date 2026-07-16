#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#include "Transform.h"
#include "JsonLoader.h"
#include <random>
#pragma once

struct AppContext;

// 罠の情報を格納するための構造体
struct MoveObjectStruct
{
    std::unique_ptr<Object3d> object;
    Transform start;
    MoveObjectData trapData;
    bool reverse;
    float startFrame;
    int number;
};

/// <summary>
/// トラップ
/// </summary>
class MoveObject
{
public:
    MoveObject();
    ~MoveObject();

    void SetContext(AppContext& context) { m_pContext = &context; }

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="jsonName">JsonLoaderに登録した名前</param>
    void Initialize(std::string jsonName);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// スタートシーン用の高さカリング用Setter
    /// </summary>
    void SetDrawHeight(const float height);

private:
    AppContext* m_pContext = nullptr;

    /// <summary>
    /// トラップを作成する関数
    /// </summary>
    void MakeMoveObject(MoveObjectStruct& data);

    std::string m_jsonName;

    std::vector<MoveObjectStruct> m_moveObjects;

    float m_gameTimer = 0.0f;

    bool m_start = false;

    std::mt19937 m_randomEngine;
    int m_num = 0;
};

