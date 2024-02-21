#pragma once
#include "../Types/DirectX/DXHelper.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};
using TDescPairRef = pair<CD3DX12_CPU_DESCRIPTOR_HANDLE&, CD3DX12_GPU_DESCRIPTOR_HANDLE&>;

struct SDescriptorPair
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
};

struct TDescriptorHandle
{
	void Offset(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPUHandle)
	{
		OutCPUHandle = CPUHandle;
		OutGPUHandle = GPUHandle;
		GPUHandle.Offset(1);
		CPUHandle.Offset(1);
	}

	void Offset(SDescriptorPair& OutHandle)
	{
		Offset(OutHandle.CPUHandle, OutHandle.GPUHandle);
	}

	void Offset(vector<SDescriptorPair>& OutHandle, uint32_t NumDesc)
	{
		for (uint32_t i = 0; i < NumDesc; i++)
		{
			auto [cpu, gpu] = Offset();
			if (i >= OutHandle.size())
			{
				OutHandle.push_back({ cpu, gpu });
			}
			else
			{
				OutHandle[i] = { cpu, gpu };
			}
		}
	}

	void Offset(const vector<TDescPairRef>& OutHandle)
	{
		for (auto& handle : OutHandle)
		{
			Offset(handle.first, handle.second);
		}
	}

	SDescriptorPair Offset(uint32_t Value = 1)
	{
		const auto oldCPU = CPUHandle;
		const auto oldGPU = GPUHandle;
		CPUHandle.Offset(Value, DescSize);
		GPUHandle.Offset(Value, DescSize);
		return { oldCPU, oldGPU };
	}

	uint32_t DescSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
};

struct SRenderObjectDescriptor : IDescriptor
{
	void Init(ID3D12DescriptorHeap* SRVHeap, ID3D12DescriptorHeap* DSVHeap, ID3D12DescriptorHeap* RTVHeap, UINT SRVSize, UINT RTVSize, UINT DSVSize);

	TDescriptorHandle SRVHandle;
	TDescriptorHandle RTVHandle;
	TDescriptorHandle DSVHandle;
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void BuildDescriptors(IDescriptor* Descriptor){};
	virtual uint32_t GetNumSRVRequired() const = 0;
	virtual uint32_t GetNumRTVRequired() { return 0; }
	virtual uint32_t GetNumDSVRequired() { return 0; }
};