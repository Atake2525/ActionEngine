#include "json.hpp"
#pragma once

namespace Setting {
    struct AudioSetting
    {
        float masterVolume = 1.0f;
        float musicVolume = 1.0f;
        float sfxVolume = 1.0f;
        bool monaural = false;
    };
    namespace AudioConfig {
        AudioSetting Load(nlohmann::json json);
        bool Save(std::string fileName, AudioSetting audioSetting);
    };
};