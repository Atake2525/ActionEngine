#include "ModelManager.h"
#include "Model.h"
#include "ModelBase.h"
#include "DirectXBase.h"

ModelManager* ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXBase* directxBase) { 
	ModelBase::GetInstance()->Initialize(directxBase); 
}

void ModelManager::LoadModel(const std::string& directoryPath, const std::string& filePath, const bool& enableLighting, const bool isAnimation) {
	// ディレクトリの最後の名前もモデルのkeyに入れる
	// Pathの長さ
	size_t pathLen = directoryPath.size();
	// directoryPathの何文字目から見るかを示す値
	size_t pathNum = 0;
	for (size_t i = directoryPath.size(); i > 1; --i)
	{
		char c = directoryPath[i - 1];
		if (c == '/') {
			pathNum = i;
			pathLen = directoryPath.size() - i;
			break;
		}
	}
	std::string filename;
	for (size_t i = 0; i < pathLen; i++)
	{
		char c = directoryPath[pathNum + i];
		filename += c;
	}

	filename = filename + '/' + filePath;

	// 読み込み済モデルを検索
	if (models.contains(filename)) {
		// 読み込み済なら早期return
		return;
	}

	// モデルの生成と読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(directoryPath, filePath, enableLighting, isAnimation);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filename, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath) {
	// 読み込み済モデルを検索
	if (models.contains(filePath)) {
	// 読み込みモデルを戻り値としてreturn
		return models.at(filePath).get();
	}

	// ファイル名一致無し
	return nullptr;
}