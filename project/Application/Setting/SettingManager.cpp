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
	if (json[keyConfigKey].is_null()) {
		Log("mainキーコンフィグがnullのため、defaultのキーコンフィグを使用します\n");
        keyConfigKey = "default";
	}

	for (nlohmann::json& bind : json[keyConfigKey])
	{
		for (auto key : ActionNameToEnum)
		{
			for (nlohmann::json& keyType : bind[key.first])
			{
				if (keyType.is_null()) // キーが未設定だったらスキップ
				{
					continue;
				}
                std::string keyTypeStr = keyType.get<std::string>();
				// キーボード,コントローラー,DPadの順で確認する
				if (keyTypeStr == "keyboard")
				{
					m_keyboardConfig.SetMainAction(key.second, ConvertKeyToDIK(bind[key.first].get<std::string>()));
				}
				if (keyTypeStr == "gamepad")
				{
                    // コントローラーのキーコンフィグは、コントローラーのキーコンフィグとDPadのキーコンフィグの両方を確認する
					if (ConvertKeyToController(bind[key.first].get<std::string>()) != Controller::None)
					{
						m_controllerConfig.SetAction(key.second, ConvertKeyToController(bind[key.first].get<std::string>()));
					}
					else
					{
						m_controllerConfig.SetAction(key.second, ConvertKeyToDPad(bind[key.first].get<std::string>()));
					}
				}
				/*if (keyTypeStr == "mouse") マウスのキーコンフィグは未実装
				{

				}*/
			}
		}
	}
}

