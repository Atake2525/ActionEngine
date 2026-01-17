#pragma once

#include "Transform.h"
#include "json.hpp"
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <map>
#include <memory>
#include "Vector2.h"

struct Children
{
	std::string type;
	std::string name;
	std::string masterName;

	Transform transform;

	std::string	file_name;
};

struct TrapData
{
	bool move;

	Transform velocity;

	bool loop;
	bool reverse;

	float runTime;

	bool spawner;
	float spawnTime;
	Vector2 spawnerTime;
};

struct JsonData
{
	std::string type;
	std::string name;

	Transform transform;
	TrapData trap;

	std::string	file_name;

	std::vector<Children> children;
};

struct LevelData
{
	std::string name;

	std::map<std::string, JsonData> datas;
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

    /// <summary>
    /// 指定した名前のJSONが読み込まれているかを確認します。
    /// </summary>
    /// <param name="jsonName">確認対象のJSONの名前または識別子を表す文字列。</param>
    /// <returns>指定したJSONが読み込まれている場合はtrue、そうでない場合はfalse。</returns>
    bool CheckJsonLoaded(const std::string& jsonName);

	/// <summary>
	/// 指定したJSONの削除
	/// </summary>
    void DeleteJson(const std::string& jsonName);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="path">Jsonのパス</param>
	/// <param name="jsonName">保存するJson名</param>
	/// <param name="Overwrite">上書きするか</param>
	void LoadJson(const std::string& path, const std::string& jsonName, const bool Overwrite = false);

	void SerchTransformFunctional(const std::string& jsonName, const std::string file_name, std::function<void(Transform transform)> function);

	const std::vector<JsonData> GetJsonData(const std::string& jsonName, const std::string file_name);

private:

	std::map<std::string, LevelData> levelDatas;

};

