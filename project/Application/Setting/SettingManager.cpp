#include "SettingManager.h"
#include "JsonLoader.h"
#include <fstream>
#include <sstream>
#include "Logger.h"


using namespace Setting;
using namespace Logger;

bool SettingManager::Load(const std::string filename, SettingType type) {
	
	// jsonを読んでデータを取得する
	nlohmann::json deserialized = JsonLoader::GetInstance()->LoadJson("Settings/" + filename);

    m_settingFileNames[static_cast<int>(type)] = filename;

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
	std::string typeKey = deserialized["type"].get<std::string>();

	if (typeKey == "KEYCONFIG")
	{
		m_keyBind = KeyConfig::Load(deserialized);
	}
	else if (typeKey == "AUDIOCONFIG")
	{
        m_audioSetting = AudioConfig::Load(deserialized);
	}
	else
	{
		Log("参照しているファイルにKEYCONFIGが存在しません");
		return false;
	}

	// 全ての処理が完了した
	return true;
}

bool SettingManager::Save(SettingType type, std::variant<Setting::KeyBind, Setting::AudioSetting> setting) {
    bool result = false;
	switch (type)
	{
	case Setting::SettingType::KeyConfig:
		result = KeyConfig::Save(m_settingFileNames[static_cast<int>(type)], std::get<Setting::KeyBind>(setting));
		break;
	case Setting::SettingType::AudioConfig:
		result = AudioConfig::Save(m_settingFileNames[static_cast<int>(type)], std::get<Setting::AudioSetting>(setting));
		break;
	}


	return result;
}
