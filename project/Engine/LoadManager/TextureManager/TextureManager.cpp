#include <cassert>
#include "TextureManager.h"
#include "DirectXBase.h"
#include "Logger.h"
#include "StringUtility.h"
#include "SrvManager.h"

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
	textureDatas.reserve(SrvManager::GetInstance()->kMaxSRVCount);

	LoadTexture("Resources/Sprite/black1x1.png");
    LoadTexture("Resources/Sprite/noise0.png");

}

void TextureManager::LoadTexture(const std::string& filePath) {

	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 早期return
		return;
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
	TextureData& textureData = textureDatas[path];

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
		textureData.metadata = image.GetMetadata();
		textureData.resource = DirectXBase::GetInstance()->CreateTextureResource(textureData.metadata);

		textureData.intermediateResource = DirectXBase::GetInstance()->UploadTextureData(textureData.resource, image);
	}

	uint32_t srvIndex = SrvManager::GetInstance()->Allocate();
	textureData.srvIndex = srvIndex;

	textureData.srvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);

	uint32_t mapIndex = GetTextureIndexByFilePath("Resources/Sprite/black1x1.png");

	textureData.normalMapSrvIndex = mapIndex;
	textureData.metallicMapSrvIndex = mapIndex;
	textureData.roughnessMapSrvIndex = mapIndex;

	SrvManager::GetInstance()->CreateSRVforTexture2D(srvIndex, textureData.resource, textureData.metadata);
}

void TextureManager::SetNormalMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	std::string path;
	size_t pathLen = targetTextureFilePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = targetTextureFilePath.size(); i > 1; --i)
	{
		char c = targetTextureFilePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = targetTextureFilePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += targetTextureFilePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[path].normalMapSrvIndex = GetTextureIndexByFilePath(filePath);
		return;
	}
}

void TextureManager::SetMetallicMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	std::string path;
	size_t pathLen = targetTextureFilePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = targetTextureFilePath.size(); i > 1; --i)
	{
		char c = targetTextureFilePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = targetTextureFilePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += targetTextureFilePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[path].metallicMapSrvIndex = GetTextureIndexByFilePath(filePath);
		return;
	}
}

void TextureManager::SetRoughnessMapTexture(const std::string& targetTextureFilePath, const std::string& filePath)
{
	std::string path;
	size_t pathLen = targetTextureFilePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = targetTextureFilePath.size(); i > 1; --i)
	{
		char c = targetTextureFilePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = targetTextureFilePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += targetTextureFilePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 早期return
		LoadTexture(filePath);
		textureDatas[path].roughnessMapSrvIndex = GetTextureIndexByFilePath(filePath);
		return;
	}
}

uint32_t TextureManager::GetnormalMapSrvIndex(const std::string& filePath)
{
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[path].normalMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

uint32_t TextureManager::GetmetallicMapSrvIndex(const std::string& filePath)
{
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[path].metallicMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

uint32_t TextureManager::GetroughnessMapSrvIndex(const std::string& filePath)
{
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[path].roughnessMapSrvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 読み込み済テクスチャを検索
	if (textureDatas.contains(path))
	{
		// 読み込み済なら要素番号を返す
		uint32_t textureIndex = textureDatas[path].srvIndex;
		return textureIndex;
	}


	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	// 範囲外指定チェック
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	assert(!textureDatas.contains(path));

	TextureData& textureData = textureDatas[path];
	return textureData.srvHandleGPU;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 範囲外指定チェック
	if (!textureDatas.contains(path))
	{
		Log("指定のtextureは見つかりませんでした");
		TextureData& textureData = textureDatas["white1x1"];
		return textureData.srvIndex;
	}

	TextureData& textureData = textureDatas[path];
	return textureData.srvIndex;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath) {
	std::string path;
	size_t pathLen = filePath.size();
	size_t pathNum = 0;
	// Pathを*.pngのみにする
	for (size_t i = filePath.size(); i > 1; --i)
	{
		char c = filePath[i - 1];
		if (c == '/')
		{
			pathNum = i;
			pathLen = filePath.size() - i;
			break;
		}
	}
	for (size_t i = 0; i < pathLen; i++)
	{
		path += filePath[pathNum + i];
	}
	// 範囲外指定違反チェック
	// 範囲外指定チェック
	assert(textureDatas.contains(path));

	TextureData& textureData = textureDatas[path];
	return textureData.metadata;
}

