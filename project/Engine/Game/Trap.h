#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#include "Transform.h"
#include "JsonLoader.h"
#include <random>
#pragma once

struct Traps
{
    std::unique_ptr<Object3d> object;
    Transform start;
    TrapData trapData;
    bool reverse;
    float startFrame;
    int number;
};

class Trap
{
public:
    Trap();
    ~Trap();

    void Initialize(std::string jsonName = "normal");
    void Update();
    void Draw();

    void SetDrawHeight(const float height);

private:

    void MakeTrap(Traps& data);

    std::vector<Traps> traps;

    float gameTimer_ = 0.0f;

    bool start = false;

    std::mt19937 randomEngine;

    int num = 0;
};

