#include "SettingManager.h"
#include "JsonLoader.h"
#include <fstream>
#include <sstream>
#include "Logger.h"


using namespace Setting;
using namespace Logger;

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
		m_keyBind = KeyConfig::Load(deserialized);
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
    bool result = false;
    result = KeyConfig::Save(filename, m_keyBind);
	return result;
}