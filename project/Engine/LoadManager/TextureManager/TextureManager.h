#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"
#include <string>
#include <wrl.h>
#include <vector>
#include <unordered_map>

enum class TextureColorSpace {
    SRGB,
    Linear
};

class DirectXBase;
class SrvManager;

#pragma once
class TextureManager {
public:
	TextureManager();
    ~TextureManager();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase& directXBase, SrvManager& srvManager);

	/// <summary>
	/// テクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	/// <returns>SRVインデックス</returns>
	uint32_t LoadTexture(const std::string& filePath, TextureColorSpace colorSpace = TextureColorSpace::SRGB);

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);

	// テクスチャのメタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

private:
    DirectXBase* m_pDirectXBase = nullptr;
    SrvManager* m_pSrvManager = nullptr;

	// テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath; // 画像のファイルパス
		DirectX::TexMetadata metadata; // 画像の幅や高さなどの情報
		Microsoft::WRL::ComPtr<ID3D12Resource> resource; // テクスチャリソース
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; // SRV作成時に必要なCPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; // 描画コマンドに必要なGPUハンドル
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;

		uint32_t normalMapSrvIndex; // 法線マップ用SRVインデックス
		uint32_t metallicMapSrvIndex; // メタリックマップ用SRVインデックス
		uint32_t roughnessMapSrvIndex; // ラフネスマップ用SRVインデックス
	};
	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas;

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

};
