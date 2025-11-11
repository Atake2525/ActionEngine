#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#include "Transform.h"
#include "JsonLoader.h"
#pragma once

enum class TrapType : uint8_t {
    Spike = 0,

};

struct Traps
{
    std::unique_ptr<Object3d> object;
    TrapType type;
    Transform start;
    TrapData trapData;
    bool reverse;
    float startFrame;
};

class Trap
{
public:
    Trap();
    ~Trap();

    void Initialize();
    void Update();
    void Draw();

private:

    std::vector<Traps> traps;

    float gameTimer_ = 0.0f;

    bool start = false;

};

