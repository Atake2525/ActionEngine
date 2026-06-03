#include <string>
#include "KeyboardConfig.h"
#include "ControllerConfig.h"
#include "json.hpp"
#include <unordered_map>
#include <optional>
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

    Setting::KeyboardConfig& GetKeyConfig() { return m_keyboardConfig; }
    Setting::ControllerConfig& GetControllerConfig() { return m_controllerConfig; }

private:

    void LoadKeyConfig(nlohmann::json json);


    Setting::KeyboardConfig m_keyboardConfig;
    Setting::ControllerConfig m_controllerConfig;

    const std::unordered_map<std::string, Action> ActionNameToEnum = {
                { "MoveForward", Action::MoveForward },
                { "MoveBack",    Action::MoveBack },
                { "MoveLeft",    Action::MoveLeft },
                { "MoveRight",   Action::MoveRight },
                { "Jump",        Action::Jump },
                { "Crouch",      Action::Crouch },
                { "Run",         Action::Run },
                { "Interact",    Action::Interact },
    };

    std::optional<Action> ToAction(const std::string& name);

};

