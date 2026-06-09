#include "AudioConfig.h"
#include "JsonLoader.h"
#include "Logger.h"

using namespace Logger;

namespace Setting {
    namespace AudioConfig {
        AudioSetting Load(nlohmann::json json) {
            AudioSetting audioSetting;

            std::string keyConfigKey = "main";
            if (!json.contains(keyConfigKey) || json[keyConfigKey].is_null()) {
                Log("mainキーコンフィグがnullのため、defaultのキーコンフィグを使用します\n");
                keyConfigKey = "default";
            }

            if (!json.contains(keyConfigKey) || !json[keyConfigKey].is_object())
            {
                Log("キーコンフィグがJsonオブジェクト型ではありません。\n");
                return audioSetting;
            }

            audioSetting.masterVolume = json[keyConfigKey]["master"].get<float>();
            audioSetting.musicVolume = json[keyConfigKey]["music"].get<float>();
            audioSetting.sfxVolume = json[keyConfigKey]["sfx"].get<float>();
            audioSetting.monaural = json[keyConfigKey]["monoral"].get<bool>();
            return audioSetting;
        }
        bool Save(std::string fileName, AudioSetting audioSetting) {
            const nlohmann::json loaded = JsonLoader::GetInstance()->LoadJson("Settings/" + fileName);

            nlohmann::ordered_json json = nlohmann::ordered_json::object();
            json["type"] = "AUDIOCONFIG";
            json["main"] = nlohmann::ordered_json::object();

            if (loaded.is_object() && loaded.contains("default"))
            {
                json["default"] = loaded["default"];
            }
            else
            {
                json["default"] = nlohmann::ordered_json::object();
            }

            json["main"]["master"] = audioSetting.masterVolume;
            json["main"]["music"] = audioSetting.musicVolume;
            json["main"]["sfx"] = audioSetting.sfxVolume;
            json["main"]["monaural"] = audioSetting.monaural;

            std::ofstream file("Settings/" + fileName);
            if (!file.is_open()) // ファイルが開けなかった場合はエラーを返す
            {
                return false;
            }

            file << json.dump(4);

            return true;
        }
    };
};