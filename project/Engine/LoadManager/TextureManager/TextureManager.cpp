#include <cassert>
#include "TextureManager.h"
#include "DirectXBase.h"
#include "Logger.h"
#include "StringUtility.h"
#include "SrvManager.h"
#include <filesystem>

using namespace Logger;
using namespace StringUtility;
using namespace Microsoft::WRL;

TextureManager* TextureManager::instance = nullptr;

// ImGuiで0番を、PostEffectで1番を使用するため、2番から使用
uint32_t TextureManager::kSRVIndexTop = 2;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::Initialize() {
	// SRVの数と同数
	textureDatas.reserve(SrvManager::GetInstance()->maxSRVCount);

	LoadTexture("Resources/Sprite/black1x1.png");
	LoadTexture("Resources/Sprite/noise0.png");

}

uint32_t TextureManager::LoadTexture(const std::string& filePath) {

	std::string fn = std::filesystem::path(filePath).filename().string();
	Log(fn + "を読み込みます\n");
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 早期return
		return textureDatas[filePath].srvIndex;
	}

	// テクスチャ枚数上限チェック
	assert(SrvManager::GetInstance()->CheckAllocate());


	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr;
	// .ddsかそうでないかを判定する
	if (filePathW.ends_with(L".dds")) // .ddsで終わって居たらddsとみなす。より安全な方法はいくらでもあるので余裕があれば対応させる
	{
		hr = DirectX::LoadFromDDSFile(filePathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else
	{
		hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	// DirectXTexでは直接的に圧縮フォーマットのMipMap生成に対応していないので、圧縮されていたらそのままimageを使うように変更する。リンク先にあるようにDecompress/Compressで対応しても良い
	DirectX::ScratchImage mipImages{};

	// 追加したテクスチャデータの差印象を取得する
	TextureData& textureData = textureDatas[filePath];

	if (DirectX::IsCompressed(image.GetMetadata().format)) // 圧縮フォーマットかどうかを調べる
	{
		mipImages = std::move(image); // 圧縮フォーマットならそのまま使うのでmoveする
		// テクスチャデータをtextureDatasの末尾に追加する
		textureData.filePath = filePath;
		textureData.metadata = mipImages.GetMetadata();
		textureData.resource = DirectXBase::GetInstance()->CreateTextureResource(textureData.metadata);

		textureData.intermediateResource = DirectXBase::GetInstance()->UploadTextureData(textureData.resource, mipImages);
	}
	else
	{
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
		// テクスチャデータをtextureDatasの末尾に追加する
		textureData.filePath = filePath;
		textureData.metadata = SUCCEEDED(hr) ? mipImages.GetMetadata() : image.GetMetadata();
		textureData.resource = DirectXBase::GetInstance()->CreateTextureResource(textureData.metadata);

		textureData.intermediateResource = DirectXBase::GetInstance()->UploadTextureData(textureData.resource, SUCCEEDED(hr) ? mipImages : image);
	}

	uint32_t srvIndex = SrvManager::GetInstance()->Allocate();
	textureData.srvIndex = srvIndex;

	textureData.srvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);

	uint32_t mapIndex = textureDatas[filePath].srvIndex;

	textureData.normalMapSrvIndex = mapIndex;
	textureData.metallicMapSrvIndex = mapIndex;
	textureData.roughnessMapSrvIndex = mapIndex;

	SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex, textureData.resource, textureData.metadata);
	return mapIndex;
}

void TextureManager::SetNormalMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[filePath].normalMapSrvIndex = textureDatas[filePath].srvIndex;
		return;
	}
}

void TextureManager::SetMetallicMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[filePath].metallicMapSrvIndex = textureDatas[filePath].srvIndex;
		return;
	}
}

void TextureManager::SetRoughnessMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[filePath].roughnessMapSrvIndex = textureDatas[filePath].srvIndex;
		return;
	}
}

uint32_t TextureManager::GetnormalMapSrvIndex(const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[filePath].normalMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

uint32_t TextureManager::GetmetallicMapSrvIndex(const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[filePath].metallicMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

uint32_t TextureManager::GetroughnessMapSrvIndex(const std::string& filePath)
{
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(filePath))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[filePath].roughnessMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {

	assert(!textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	// 範囲外指定チェック
	if (!textureDatas.contains(filePath))
	{
		Log("指定のtextureは見つかりませんでした");
		TextureData& textureData = textureDatas["white1x1"];
		return textureData.srvIndex;
	}

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvIndex;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	// 範囲外指定違反チェック
	// 範囲外指定チェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}

