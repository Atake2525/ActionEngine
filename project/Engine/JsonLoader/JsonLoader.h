#pragma once

#include "json.hpp"
#include <fstream>
#include <string>

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

	nlohmann::json LoadJson(const std::string fullPath);
};

