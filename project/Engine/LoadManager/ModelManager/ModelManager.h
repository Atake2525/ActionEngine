#include <map>
#include <string>
#include <memory>
#include <unordered_map>

#pragma once

class Model;

class ModelManager {

	// 初期化
	void Initialize();

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"> : ディレクトリ(元ファイル)のパス</param>
	/// <param name="fileName"> : モデルファイルの名前</param>
	/// <param name="enableLighting"> : ライティングを適用するかどうか</param>
	/// enableLightingは何も入力しなければfalse
	Model* LoadModel(const std::string& directoryPath, const std::string& fileName, const bool isAnimation = false);

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="fileName">モデルファイルの名前</param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& fileName);

private:
	// モデレータ
	std::unordered_map<std::string, std::unique_ptr<Model>> m_models;
};
