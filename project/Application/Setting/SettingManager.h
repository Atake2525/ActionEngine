#include <string>
#include "KeyboardConfig.h"
#include "ControllerConfig.h"
#include "json.hpp"
#include <unordered_map>
#include <optional>
#include "SettingUtility.h"
#pragma once

class SettingManager
{
public:

    // Jsonファイルから設定を読み込む
    bool Load(const std::string filename);
    // 設定をJsonファイルに保存する
    bool Save(const std::string filename);

    // キーコンフィグとコントローラーコンフィグを取得する
    Setting::KeyboardConfig& GetKeyConfig() { return m_keyboardConfig; }
    Setting::ControllerConfig& GetControllerConfig() { return m_controllerConfig; }
    Setting::Sensitivity& GetSensitivity() { return m_sensitivity; }

private:

    void LoadKeyConfig(nlohmann::json json);

    Setting::KeyboardConfig m_keyboardConfig;
    Setting::ControllerConfig m_controllerConfig;
    Setting::Sensitivity m_sensitivity;
};

