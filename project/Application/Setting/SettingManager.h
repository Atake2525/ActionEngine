#include <string>
#include <unordered_map>
#include <optional>
#include "SettingUtility.h"
#include "KeyConfig.h"
#include "AudioConfig.h"
#include <variant>
#pragma once

class JsonLoader;

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
    void SetContext(JsonLoader& jsonLoader) { m_pJsonLoader = &jsonLoader; }

    // Jsonファイルから設定を読み込む
    bool Load(const std::string& fileName, Setting::SettingType type);
    // 一部設定をJsonファイルに保存する
    bool Save(Setting::SettingType type, std::variant<Setting::KeyBind, Setting::AudioSetting> setting);

    // キーコンフィグとコントローラーコンフィグを取得する
    Setting::KeyBind& GetKeyConfig() { return m_keyBind; }
    Setting::AudioSetting& GetAudioSetting() { return m_audioSetting; }

private:
    JsonLoader* m_pJsonLoader = nullptr;
    Setting::KeyBind m_keyBind;
    Setting::AudioSetting m_audioSetting;

    std::array<std::string, 2> m_settingFileNames = {};
};

