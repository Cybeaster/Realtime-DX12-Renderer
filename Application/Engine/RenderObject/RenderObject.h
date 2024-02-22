#pragma once
#include "../Types/DirectX/DXHelper.h"
#include "Logger.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};
using TDescPairRef = pair<CD3DX12_CPU_DESCRIPTOR_HANDLE&, CD3DX12_GPU_DESCRIPTOR_HANDLE&>;

struct SDescriptorPair
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle = {};
};

struct SDescriptorResourceData
{
	ID3D12DescriptorHeap* Heap;
	UINT Size;
	UINT Count;
};

struct TDescriptorHandle
{
	void Offset(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPUHandle)
	{
		CurrentOffset++;
		OutCPUHandle = CPUHandle;
		OutGPUHandle = GPUHandle;
		CPUHandle.Offset(1, DescSize);

		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(1, DescSize);
		}
		Check();
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
		CurrentOffset += Value;
		const auto oldCPU = CPUHandle;
		const auto oldGPU = GPUHandle;
		CPUHandle.Offset(Value, DescSize);
		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(Value, DescSize);
		}
		Check();
		return { oldCPU, oldGPU };
	}
	void Check() const
	{
		if (CurrentOffset > MaxOffset)
		{
			LOG(Render, Error, "Descriptor heap overflow");
		}

		if (!bIsInitized)
		{
			LOG(Render, Error, "Descriptor heap not initized");
		}

		if (MaxOffset == 0)
		{
			LOG(Render, Error, "Descriptor heap max count not initized");
		}
	}

	void Init(SDescriptorResourceData Data)
	{
		bIsInitized = true;
		MaxOffset = Data.Count;
		DescSize = Data.Size;
		CPUHandle = Data.Heap->GetCPUDescriptorHandleForHeapStart();
		if (Data.Heap->GetDesc().Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			GPUHandle = Data.Heap->GetGPUDescriptorHandleForHeapStart();
		}
	}

	bool bIsInitized = false;
	uint32_t DescSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
	uint32_t CurrentOffset = 0;
	uint32_t MaxOffset = 0;
};

struct SRenderObjectDescriptor : IDescriptor
{
	void Init(SDescriptorResourceData SRV, SDescriptorResourceData RTV, SDescriptorResourceData DSV);

	TDescriptorHandle SRVHandle;
	TDescriptorHandle RTVHandle;
	TDescriptorHandle DSVHandle;
};

class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void BuildDescriptors(IDescriptor* Descriptor){};
	virtual void Init(){};
	virtual uint32_t GetNumSRVRequired() const = 0;
	virtual uint32_t GetNumRTVRequired() { return 0; }
	virtual uint32_t GetNumDSVRequired() { return 0; }
};