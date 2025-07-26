#include "SrvManager.h"
#include "Logger.h"
#include "StringUtility.h"
#include "format"
#include <cassert>
#include "DirectXBase.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;
using namespace Logger;
using namespace StringUtility;

// 最大テクスチャ枚数
const uint32_t SrvManager::kMaxSRVCount = 512;

SrvManager* SrvManager::instance = nullptr;

SrvManager* SrvManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SrvManager;
	}
	return instance;
}

void SrvManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void SrvManager::Initialize(DirectXBase* directxBase) {
	directxBase_ = directxBase;
	CreateDescriptorHeap();
}

void SrvManager::CreateDescriptorHeap() {
	// でスクリプタヒープの生成
	descriptorHeap = directxBase_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	// デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directxBase_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate() {
	// 上限に達していないかチェックしてassert
	assert(useIndex < kMaxSRVCount);

	// returnする番号を一旦記録しておく
	int index = useIndex;
	// 次回のために番号を1進める
	useIndex++;
	// 上で記録した番号をreturn
	return index;
}

bool SrvManager::CheckAllocate()
{
	if (useIndex > kMaxSRVCount)
	{
		return false;
	}
	else
	{
		return true;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DirectX::TexMetadata metadata)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	// SRVの設定を行う
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (metadata.IsCubemap())
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
		srvDesc.Texture2D.MipLevels = metadata.mipLevels;
	}

	directxBase_->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, GetCPUDescriptorHandle(srvIndex));

}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DirectX::TexMetadata metaData, D3D12_SRV_DIMENSION dimension)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	// SRVの設定を行う
	srvDesc.Format = metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = dimension;

	if (dimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
		srvDesc.Texture2D.MipLevels = metaData.mipLevels;
	}

	directxBase_->GetDevice()->CreateShaderResourceView(pResource.Get(), &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, UINT numElements, UINT structureByteStride)
{
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	directxBase_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw() {
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	directxBase_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}
