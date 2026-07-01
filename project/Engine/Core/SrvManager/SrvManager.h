#include "WinApp.h"
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <array>
#include <dxcapi.h>
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#pragma once

class DirectXBase;

class SrvManager
{
public:
    // 最大SRV数(最大テクスチャ枚数)
    static const uint32_t maxSRVCount;
    static const uint32_t maxUAVCount;

    void PreDraw();

    // 初期化
    void Initialize(DirectXBase& directXBase);

    uint32_t Allocate();

    const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap; }

    bool CheckAllocate();

    /// <summary>
    /// SRVの指定番号のCPUデスクリプタハンドルを取得する
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);

    /// <summary>
    /// SRVの指定番号のGPUデスクリプタハンドル
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

    // SRV生成(テクスチャ用)
    void CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DirectX::TexMetadata metaData);
    void CreateSRVforTexture2D(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, DirectX::TexMetadata metaData, D3D12_SRV_DIMENSION dimension);
    // SRV生成(Structured Buffer用)
    void CreateSRVforStructuredBuffer(uint32_t srvIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, UINT numElements, UINT structureByteStride);
    void CreateUAVforStructuredBuffer(uint32_t uavIndex, Microsoft::WRL::ComPtr<ID3D12Resource> pResource, UINT numElements, UINT structureByteStride);

    void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
    void SetComputeRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);


private:
    void CreateDescriptorHeap();

    DirectXBase* m_pDirectXBase;

    // SRV用のデスクリプタサイズ
    uint32_t descriptorSize;
    // SRV様のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、shaderVisibleはtrue
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

private:
    // 次に使用するSRVインデックス
    uint32_t useIndex = 0;
};

