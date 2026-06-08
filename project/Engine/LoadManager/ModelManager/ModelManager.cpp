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

void ModelManager::Initialize() { 
	ModelBase::GetInstance()->Initialize(); 
}

Model* ModelManager::LoadModel(const std::string& directoryPath, const std::string& fileName, const bool isAnimation) {
	std::string filename = fileName;

	// 読み込み済モデルを検索
	if (models.contains(filename)) {
		// 読み込み済なら早期return
		return models.at(filename).get();
	}

	// モデルの生成と読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(directoryPath, fileName, isAnimation);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filename, std::move(model)));
	return models.at(filename).get();
}

Model* ModelManager::FindModel(const std::string& fileName) {
	// 読み込み済モデルを検索
	if (models.contains(fileName)) {
	// 読み込みモデルを戻り値としてreturn
		return models.at(fileName).get();
	}

	// ファイル名一致無し
	return nullptr;
}