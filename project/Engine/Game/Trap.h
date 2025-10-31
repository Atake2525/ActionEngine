#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#pragma once

enum class TrapType : uint8_t {
    Spike = 0,

};

struct Traps
{
    std::unique_ptr<Object3d> object;
    TrapType type;
};

class Trap
{
public:
    void Initialize(std::string path);
    void Update();
    void Draw();

private:

    std::vector<Traps> traps;

    void UpdateSpikeTrap();

};

