#include <string>
#include <unordered_map>
#include <optional>
#include "SettingUtility.h"
#include "KeyConfig.h"
#pragma once

class SettingManager
{
public:

    // Jsonファイルから設定を読み込む
    bool Load(const std::string filename);
    // 設定をJsonファイルに保存する
    bool Save(const std::string filename);

    // キーコンフィグとコントローラーコンフィグを取得する
    Setting::KeyboardConfig& GetKeyConfig() { return m_keyBind.keyboardConfig; }
    Setting::ControllerConfig& GetControllerConfig() { return m_keyBind.controllerConfig; }
    Setting::Sensitivity& GetSensitivity() { return m_keyBind.sensitivity; }

private:
    Setting::KeyBind m_keyBind;
};

