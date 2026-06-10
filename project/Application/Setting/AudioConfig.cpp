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
            audioSetting.monaural = json[keyConfigKey]["monaural"].get<bool>();
            return audioSetting;
        }
        bool Save(std::string fileName, AudioSetting audioSetting) {

            AudioSetting settings = audioSetting;
            bool makedFile = false;
            if (!std::filesystem::exists("Settings/" + fileName)) //ファイルが存在しないのでdefaultの値を保存する
            {
                Log("ファイルの展開に失敗、またはファイルが存在しないため、defaultの設定を保存します\n");
                settings = AudioSetting();
                makedFile = true;
            }

            nlohmann::ordered_json json = nlohmann::ordered_json::object();
            json["type"] = "AUDIOCONFIG";
            json["main"] = nlohmann::ordered_json::object();

           

            json["main"]["master"] = settings.masterVolume;
            json["main"]["music"] = settings.musicVolume;
            json["main"]["sfx"] = settings.sfxVolume;
            json["main"]["monaural"] = settings.monaural;

            std::ofstream file("Settings/" + fileName);

            if (!file.is_open() && !makedFile) {
                Log("ファイルの保存に失敗しました: " + fileName + "\n");
                return false;
            }       

            file << json.dump(4);

            return true;
        }
    };
};