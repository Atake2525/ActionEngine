#pragma once

#include "Transform.h"
#include "Vector2.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct LevelEditorChildData
{
	std::string type;
	std::string name;
	std::string masterName;

	Transform transform;

	std::string	file_name;
};

struct TrapData
{
	bool move = false;

	Transform velocity = Transform::Default;

	bool loop = false;
	bool reverse = false;

	float runTime = 1.0f;

	bool spawner = false;
	float spawnTime = 0.0f;
	Vector2 spawnerTime = { 0.0f, -1.0f };
};

struct LevelEditorData
{
	std::string type;
	std::string name;

	Transform transform;
	TrapData trap;

	std::string	file_name;

	std::vector<LevelEditorChildData> children;
};

struct LevelData
{
	std::string name;

	std::unordered_map<std::string, LevelEditorData> datas;
};

class LevelEditor {
public:
	void Initialize();

	bool CheckJsonLoaded(const std::string& jsonName) const;

	void DeleteJson(const std::string& jsonName);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="path">Jsonのパス</param>
	/// <param name="jsonName">保存するJson名</param>
	/// <param name="Overwrite">上書きするか</param>
	void LoadJson(const std::string& path, const std::string& jsonName, const bool Overwrite = false);

	void SerchTransformFunctional(const std::string& jsonName, const std::string fileName, std::function<void(Transform transform)> function) const;

	std::vector<LevelEditorData> GetJsonData(const std::string& jsonName, const std::string file_name) const;

private:
	std::unordered_map<std::string, std::vector<LevelEditorData>> m_levelDatas;
};
