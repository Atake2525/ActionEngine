#include "KeyConfig.h"
#include "InputConverter.h"
#include "JsonLoader.h"
#include "Logger.h"

using namespace Logger;
using namespace InputConverter;

namespace Setting {
    namespace KeyConfig {
        KeyBind Load(nlohmann::json json) {
            KeyBind keyBind;
            // mainキーコンフィグの確認
            // mainキーがnullだったらdefaultのキーコンフィグを使用する
            std::string keyConfigKey = "main";
            if (!json.contains(keyConfigKey) || json[keyConfigKey].is_null()) {
                Log("mainキーコンフィグがnullのため、defaultのキーコンフィグを使用します\n");
                keyConfigKey = "default";
            }

            if (!json.contains(keyConfigKey) || !json[keyConfigKey].is_object())
            {
                Log("キーコンフィグがJsonオブジェクト型ではありません。\n");
                return keyBind;
            }

            const nlohmann::json& keyConfig = json[keyConfigKey];

            for (auto it = keyConfig.begin(); it != keyConfig.end(); ++it)
            {
                if (it.key() == "sensitivity") // 感度設定
                {
                    keyBind.sensitivity.mouse = it.value()["mouse"].get<float>();
                    keyBind.sensitivity.controller = it.value()["controller"].get<float>();
                    keyBind.sensitivity.invertX = it.value()["invertX"].get<bool>();
                    keyBind.sensitivity.invertY = it.value()["invertY"].get<bool>();
                    continue;
                }
                // キーコンフィグのキーがActionに変換できない場合はスキップする
                std::optional<Action> action = ToAction(it.key());
                if (!action.has_value())
                {
                    continue;
                }

                // キーコンフィグの値がオブジェクト型でない場合はスキップする
                const nlohmann::json& bind = it.value();
                if (!bind.is_object())
                {
                    continue;
                }

                // キーボードのキー設定
                if (bind.contains("keyboard") && bind["keyboard"].is_string())
                {
                    keyBind.keyboardConfig.SetMainAction(action.value(), ConvertKeyToDIK(bind["keyboard"].get<std::string>()));
                }

                if (bind.contains("gamepad") && bind["gamepad"].is_string())
                {
                    const std::string gamepadKey = bind["gamepad"].get<std::string>();

                    Controller controller = ConvertKeyToController(gamepadKey);
                    if (controller != Controller::None)
                    {
                        keyBind.controllerConfig.SetAction(action.value(), controller);
                    }

                    DPad dpad = ConvertKeyToDPad(gamepadKey);
                    if (dpad != DPad::None)
                    {
                        keyBind.controllerConfig.SetAction(action.value(), dpad);
                    }

                    StickDirection stickDirection = ConvertKeyToStickDirection(gamepadKey);
                    if (stickDirection != StickDirection::None)
                    {
                        keyBind.controllerConfig.SetAction(action.value(), stickDirection);
                    }
                }
            }
            return keyBind;
        }
        bool Save(std::string fileName, KeyBind keyBind) {
            
            nlohmann::ordered_json json = JsonLoader::GetInstance()->LoadJson("Settings/" + fileName);

            if (!json.is_object())
            {
                json = nlohmann::ordered_json::object();
            }

            json["type"] = "KEYCONFIG";
            json["main"] = nlohmann::ordered_json::object();
            json["default"] = json["default"];

            // 各 Action を main に書き込む
            // keyboard は ConvertDIKToKey()
            // gamepad は Controller / DPad / StickDirection のどれかを文字列へ戻す
            // mouse は現状ロードで使っていないので null
            for (const auto& action : Setting::ActionNameToEnum)
            {
                nlohmann::ordered_json bind;
                bind["keyboard"] = ConvertDIKToKey(
                    static_cast<BYTE>(keyBind.keyboardConfig.GetMainAction(action.second))
                );
                bind["mouse"] = nullptr;

                std::string gamepad = "null";

                StickDirection stick = keyBind.controllerConfig.GetStickAction(action.second);
                DPad dpad = keyBind.controllerConfig.GetDPadAction(action.second);
                Controller controller = keyBind.controllerConfig.GetControllerAction(action.second);

                if (stick != StickDirection::None) {
                    gamepad = ConvertStickDirectionToKey(stick);
                }
                else if (dpad != DPad::None) {
                    gamepad = ConvertDPadToKey(dpad);
                }
                else if (controller != Controller::None) {
                    gamepad = ConvertControllerToKey(controller);
                }

                bind["gamepad"] = gamepad == "null" ? nlohmann::ordered_json(nullptr) : nlohmann::ordered_json(gamepad);
                json["main"][action.first] = bind;
            }

            std::ofstream file("Settings/" + fileName);
            if (!file.is_open())
            {
                return false;
            }

            file << json.dump(4);
            return true;
        }
    };
};