#include "json.hpp"
#include "KeyboardConfig.h"
#include "ControllerConfig.h"
#include "SettingUtility.h"

#pragma once

namespace Setting {
    struct KeyBind
    {
        Setting::KeyboardConfig keyboardConfig;
        Setting::ControllerConfig controllerConfig;
        Setting::Sensitivity sensitivity;
    };
    namespace KeyConfig {
        KeyBind Load(nlohmann::json json);
        bool Save(std::string fileName, KeyBind keyBind);
    };
};