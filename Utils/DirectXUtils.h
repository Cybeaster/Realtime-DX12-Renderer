#pragma once
#include "DirectX/DXHelper.h"

#include <Windows.h>
#include <d3dx12.h>

#include <array>

namespace Utils
{
UINT CalcBufferByteSize(const UINT ByteSize);

ComPtr<ID3DBlob> CompileShader(const std::wstring& FileName,
                               const D3D_SHADER_MACRO* Defines,
                               const std::string& EntryPoint,
                               const std::string& Target);

ComPtr<ID3DBlob> LoadBinary(const wstring& FileName);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* Device,
    ID3D12GraphicsCommandList* CommandList,
    const void* InitData,
    UINT64 ByteSize,
    ComPtr<ID3D12Resource>& UploadBuffer);

array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
void ResourceBarrier(ID3D12GraphicsCommandList* CMDList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
D3D12_RESOURCE_STATES ResourceBarrier(ID3D12GraphicsCommandList* CMDList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES After);
void BuildRootSignature(ID3D12Device* Device, ComPtr<ID3D12RootSignature>& RootSignature, const D3D12_ROOT_SIGNATURE_DESC& Desc);
} // namespace Utils
