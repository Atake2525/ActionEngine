#pragma once

#include "Transform.h"
#include "json.hpp"
#include <sstream>
#include <fstream>

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

class LoadJson
{
private:
	// シングルトンパターンを適用
	static LoadJson* instance;

	// コンストラクタ、デストラクタの隠蔽
	LoadJson() = default;
	~LoadJson() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	LoadJson(LoadJson&) = delete;
	LoadJson& operator=(LoadJson&) = delete;

public:
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static LoadJson* GetInstance();

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

