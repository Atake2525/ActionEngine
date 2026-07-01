#include <map>
#include <string>
#include <memory>
#include <unordered_map>

#pragma once

class Model;
class DirectXBase;
class SrvManager;
class TextureManager;
class Object3dBase;
class SkyBox;

class ModelManager {
public:
	void SetContext(DirectXBase& directXBase, SrvManager& srvManager, TextureManager& textureManager, Object3dBase& object3dBase, SkyBox& skyBox);

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
	DirectXBase* m_pDirectXBase = nullptr;
	SrvManager* m_pSrvManager = nullptr;
	TextureManager* m_pTextureManager = nullptr;
	Object3dBase* m_pObject3dBase = nullptr;
	SkyBox* m_pSkyBox = nullptr;

	// モデレータ
	std::unordered_map<std::string, std::unique_ptr<Model>> m_models;
};
