#include "Object3d.h"
#include <string>
#include <map>
#include <memory>
#pragma once

class Trap
{
public:
    void Initialize();
    void Update();
    void Draw();

private:

    std::map<std::wstring, std::unique_ptr<Object3d>> traps;

};

