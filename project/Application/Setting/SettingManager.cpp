#include "SettingManager.h"
#include "JsonLoader.h"
#include <fstream>
#include <sstream>
#include "Logger.h"
#include "InputConverter.h"

using namespace Setting;
using namespace Logger;
using namespace InputConverter;

bool SettingManager::Load(const std::string filename) {
	
	// jsonを読んでデータを取得する
	nlohmann::json deserialized = JsonLoader::GetInstance()->LoadJson("Settings/" + filename);

	// 正しいレベルデータファイルかチェック
	if (!deserialized.is_object())
	{
		Log("参照しているファイルがJsonオブジェクト型ではありません。\n");
		return false;
	}
	if (!deserialized.contains("type"))
	{
		Log("参照しているファイルにtypeが存在しません\n");
		return false;
	}
	if (!deserialized["type"].is_string())
	{
		Log("typeが文字列ではありません\n");
		return false;
	}

	// typeを参照してなんの設定項目なのかを判断する
	std::string type = deserialized["type"].get<std::string>();

	if (type == "KEYCONFIG")
	{
		LoadKeyConfig(deserialized);
	}
	else
	{
		Log("参照しているファイルにKEYCONFIGが存在しません");
		return false;
	}

	// 全ての処理が完了した
	return true;
}

bool SettingManager::Save(const std::string filename) {
	return true;
}

void SettingManager::LoadKeyConfig(nlohmann::json json) {
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
		return;
	}

	const nlohmann::json& keyConfig = json[keyConfigKey];
	/*for (auto bind : keyConfig)
	{
		std::optional<Action> action = ToAction(bind.get<std::string>());
		if (!action.has_value())
		{
			continue;
		}
	}*/
	for (auto it = keyConfig.begin(); it != keyConfig.end(); ++it)
	{
		std::optional<Action> action = ToAction(it.key());
		if (!action.has_value())
		{
			continue;
		}

		const nlohmann::json& bind = it.value();
		if (!bind.is_object())
		{
			continue;
		}

		if (bind.contains("keyboard") && bind["keyboard"].is_string())
		{
			m_keyboardConfig.SetMainAction(action.value(), ConvertKeyToDIK(bind["keyboard"].get<std::string>()));
		}

		if (bind.contains("gamepad") && bind["gamepad"].is_string())
		{
			const std::string gamepadKey = bind["gamepad"].get<std::string>();

			Controller controller = ConvertKeyToController(gamepadKey);
			if (controller != Controller::None)
			{
				m_controllerConfig.SetAction(action.value(), controller);
			}

			DPad dpad = ConvertKeyToDPad(gamepadKey);
			if (dpad != DPad::None)
			{
				m_controllerConfig.SetAction(action.value(), dpad);
			}

			StickDirection stickDirection = ConvertKeyToStickDirection(gamepadKey);
			if (stickDirection != StickDirection::None)
			{
				m_controllerConfig.SetAction(action.value(), stickDirection);
			}
		}
	}
}

