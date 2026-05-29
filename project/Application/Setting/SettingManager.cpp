#include "SettingManager.h"
#include <fstream>
#include <sstream>
#include "Logger.h"

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

void SettingManager::Load(const std::string filename) {
	// フルパスを作成
	std::string fullPath = "Resources/Json/" + filename;

	// ファイルストリーム
	std::fstream file;
	// ファイル展開
	file.open(fullPath);
	// 展開失敗のチェック
	if (file.fail())
	{
		Log("ファイル展開に失敗しました。正しい位置にファイルがあるか確認してください。/nファイルの位置 : " + fullPath);
		return;
	}

	// Json文字列から解凍したデータ
	nlohmann::json deserialized;

	// 解凍
	file >> deserialized;

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
		SetKeyConfig(deserialized);
	}
}

void SettingManager::Save(const std::string filename) {

}

void SettingManager::SetKeyConfig(nlohmann::json) {

}
