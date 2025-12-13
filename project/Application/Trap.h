#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#include "Transform.h"
#include "JsonLoader.h"
#include <random>
#pragma once

// 罠の情報を格納するための構造体
struct Traps
{
    std::unique_ptr<Object3d> object;
    Transform start;
    TrapData trapData;
    bool reverse;
    float startFrame;
    int number;
};

/// <summary>
/// トラップ
/// </summary>
class Trap
{
public:
    Trap();
    ~Trap();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="jsonName">JsonLoaderに登録した名前</param>
    void Initialize(std::string jsonName = "normal");

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

    /// <summary>
    /// トラップを作成する関数
    /// </summary>
    void MakeTrap(Traps& data);

    std::string jsonName_;

    std::vector<Traps> traps;

    float gameTimer_ = 0.0f;

    bool start = false;

    std::mt19937 randomEngine;

    int num = 0;
};

