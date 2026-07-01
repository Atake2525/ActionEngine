#include "ModelManager.h"
#include "Model.h"
#include "DirectXBase.h"
#include <cassert>

void ModelManager::SetContext(DirectXBase& directXBase, SrvManager& srvManager, TextureManager& textureManager, Object3dBase& object3dBase, SkyBox& skyBox) {
	m_pDirectXBase = &directXBase;
	m_pSrvManager = &srvManager;
	m_pTextureManager = &textureManager;
	m_pObject3dBase = &object3dBase;
	m_pSkyBox = &skyBox;
}

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
	assert(m_pDirectXBase);
	assert(m_pSrvManager);
	assert(m_pTextureManager);
	assert(m_pObject3dBase);
	assert(m_pSkyBox);
	model->SetContext(*m_pDirectXBase, *m_pSrvManager, *m_pTextureManager, *m_pObject3dBase, *m_pSkyBox);
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
