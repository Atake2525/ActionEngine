#include <string>
#include "KeyConfig.h"
#include "json.hpp"
#pragma once

class SettingManager
{
private:
    static SettingManager* instance;

    SettingManager() = default;
    ~SettingManager() = default;

    SettingManager(SettingManager&) = default;
    SettingManager& operator=(SettingManager&) = default;
public:

    static SettingManager* GetInstance();

    void Load(const std::string filename);
    void Save(const std::string filename);

    Setting::KeyConfig& GetKeyConfig() { return m_keyConfig; }

private:

    void SetKeyConfig(nlohmann::json json);

    Setting::KeyConfig m_keyConfig;

};

