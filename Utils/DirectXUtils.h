#pragma once
#include "DirectX/DXHelper.h"
#include "DirectX/Resource.h"

#include <Windows.h>
#include <d3dx12.h>

#include <array>

class OCommandQueue;
class IRenderObject;
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

vector<CD3DX12_STATIC_SAMPLER_DESC> GetStaticSamplers();
void ResourceBarrier(ID3D12GraphicsCommandList* List, SResourceInfo* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After);
void ResourceBarrier(ID3D12GraphicsCommandList* List, SResourceInfo* Resource, D3D12_RESOURCE_STATES After);

void BuildRootSignature(ID3D12Device* Device, ComPtr<ID3D12RootSignature>& RootSignature, const D3D12_ROOT_SIGNATURE_DESC& Desc);
void BuildRootSignature(ID3D12Device* Device, ComPtr<ID3D12RootSignature>& RootSignature, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Desc);

void CreateRootSignature(ID3D12Device* Device, ComPtr<ID3D12RootSignature>& RootSignature, const ComPtr<ID3DBlob>& SerializedRootSig, const ComPtr<ID3DBlob>& ErrorBlob);
DXGI_FORMAT MaskToFormat(uint32_t Mask);

SResourceInfo CreateResource(IRenderObject* Owner, ID3D12Device* Device, D3D12_HEAP_TYPE HeapProperties, const D3D12_RESOURCE_DESC& Desc, D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_GENERIC_READ, const D3D12_CLEAR_VALUE* ClearValue = nullptr);
SResourceInfo CreateResource(IRenderObject* Owner,
                             ID3D12Device* Device,
                             D3D12_HEAP_TYPE HeapProperties,
                             const D3D12_RESOURCE_DESC& Desc,
                             D3D12_RESOURCE_STATES InitialState,
                             ID3D12GraphicsCommandList* CMDList,
                             const D3D12_CLEAR_VALUE* ClearValue = nullptr);
} // namespace Utils
