#include <string>
#include <unordered_map>
#include <optional>
#include "SettingUtility.h"
#include "KeyConfig.h"
#include "AudioConfig.h"
#include <variant>
#pragma once

namespace Setting {
    enum class SettingType
    {
        KeyConfig = 0,
        AudioConfig = 1,
    };
};

class SettingManager
{
public:

    // Jsonファイルから設定を読み込む
    bool Load(const std::string filename, Setting::SettingType type);
    // 一部設定をJsonファイルに保存する
    bool Save(Setting::SettingType type, std::variant<Setting::KeyBind, Setting::AudioSetting> setting);

    // キーコンフィグとコントローラーコンフィグを取得する
    Setting::KeyBind& GetKeyConfig() { return m_keyBind; }
    Setting::AudioSetting& GetAudioSetting() { return m_audioSetting; }

private:
    Setting::KeyBind m_keyBind;
    Setting::AudioSetting m_audioSetting;

    std::array<std::string, 2> m_settingFileNames = {};
};

