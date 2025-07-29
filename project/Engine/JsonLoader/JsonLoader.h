#pragma once

#include "Transform.h"
#include "json.hpp"
#include <sstream>
#include <fstream>
#include <Windows.h>

struct Children
{
	std::string type;
	std::string name;
	std::string masterName;

	Transform transform;

	std::string	file_name;
};

struct JsonData
{
	std::string type;
	std::string name;

	Transform transform;

	std::string	file_name;

	std::vector<Children> children;
};

struct LevelData
{
	std::string name;

	std::vector<JsonData> datas;
};

struct HotReload
{
	std::string directoryPath;
	std::string filename;

	std::string fullpath;

	FILETIME& lastWriteTime;
};

class JsonLoader
{
private:
	// シングルトンパターンを適用
	static JsonLoader* instance;

	// コンストラクタ、デストラクタの隠蔽
	JsonLoader() = default;
	~JsonLoader() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	JsonLoader(JsonLoader&) = delete;
	JsonLoader& operator=(JsonLoader&) = delete;

public:
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static JsonLoader* GetInstance();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	const LevelData LoadJsonTransform(const std::string& directoryPath, const std::string& fileName);

};

