#include "SettingManager.h"
#include "JsonLoader.h"
#include <fstream>
#include <sstream>
#include "Logger.h"
#include "SettingUtility.h"

using namespace Setting;
using namespace Logger;

SettingManager* SettingManager::instance = nullptr;

SettingManager* SettingManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new SettingManager;
	}
	return instance;
}

std::optional<Action> SettingManager::ToAction(const std::string& name) {
	const auto it = ActionNameToEnum.find(name);
	if (it == ActionNameToEnum.end())
	{
		return std::nullopt;
	}

	return it->second;
}

void SettingManager::Load(const std::string filename) {
	
	// jsonを読んでデータを取得する
	nlohmann::json deserialized = JsonLoader::GetInstance()->LoadJson("Settings/" + filename);

	// 正しいレベルデータファイルかチェック
	if (!deserialized.is_object())
	{
		Log("参照しているファイルがJsonオブジェクト型ではありません。\n");
		return;
	}
	if (!deserialized.contains("type"))
	{
		Log("参照しているファイルにtypeが存在しません\n");
		return;
	}
	if (!deserialized["type"].is_string())
	{
		Log("typeが文字列ではありません\n");
		return;
	}

	// typeを参照してなんの設定項目なのかを判断する
	std::string type = deserialized["type"].get<std::string>();

	if (type == "KEYCONFIG")
	{
		LoadKeyConfig(deserialized);
	}
}

void SettingManager::Save(const std::string filename) {

}

void SettingManager::LoadKeyConfig(nlohmann::json json) {
	// mainキーコンフィグの確認
	// mainキーがnullだったらdefaultのキーコンフィグを使用する
    std::string keyConfigKey = "main";
	if (json[keyConfigKey].is_null()) {
		Log("mainキーコンフィグがnullのため、defaultのキーコンフィグを使用します\n");
        keyConfigKey = "default";
	}



}

