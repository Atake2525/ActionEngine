#include "ModelManager.h"
#include "Model.h"
#include "DirectXBase.h"

void ModelManager::Initialize() { 
}

Model* ModelManager::LoadModel(const std::string& directoryPath, const std::string& fileName, const bool isAnimation) {
	const std::string& modelKey = fileName;

	// 読み込み済モデルを検索
	if (m_models.contains(modelKey)) {
		// 読み込み済なら早期return
		return m_models.at(modelKey).get();
	}

	// モデルの生成と読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(directoryPath, fileName, isAnimation);

	// モデルをmapコンテナに格納する
	m_models.insert(std::make_pair(modelKey, std::move(model)));
	return m_models.at(modelKey).get();
}

Model* ModelManager::FindModel(const std::string& fileName) {
	// 読み込み済モデルを検索
	if (m_models.contains(fileName)) {
	// 読み込みモデルを戻り値としてreturn
		return m_models.at(fileName).get();
	}

	// ファイル名一致無し
	return nullptr;
}
