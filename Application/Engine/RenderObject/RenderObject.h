#pragma once

#include "DirectX/ObjectConstants.h"
#include "Engine/UploadBuffer/UploadBuffer.h"
#include "Events.h"
#include "Logger.h"
#include "Statics.h"

struct IDescriptor
{
	virtual ~IDescriptor() = default;
};

struct SDescriptorPair
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle = {};
	uint32_t Index = UINT32_MAX;
};

struct SDescriptorResourceData
{
	ID3D12DescriptorHeap* Heap;
	UINT Size;
	UINT Count;
};

struct TDescriptorHandle
{
	TDescriptorHandle()
	{
		CPUHandle.ptr = 0;
		GPUHandle.ptr = 0;
	}

	void Offset(CD3DX12_CPU_DESCRIPTOR_HANDLE& OutCPUHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& OutGPUHandle, uint32_t& OutIndex)
	{
		OutCPUHandle = CPUHandle;
		OutGPUHandle = GPUHandle;
		OutIndex = CurrentOffset;
		CurrentOffset++;
		CPUHandle.Offset(1, DescSize);

		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(1, DescSize);
		}
		Check();
	}

	void Offset(SDescriptorPair& OutHandle)
	{
		Offset(OutHandle.CPUHandle, OutHandle.GPUHandle, OutHandle.Index);
	}

	void Offset(vector<SDescriptorPair>& OutHandle, uint32_t NumDesc)
	{
		for (uint32_t i = 0; i < NumDesc; i++)
		{
			auto pair = Offset();
			if (i >= OutHandle.size())
			{
				OutHandle.push_back(pair);
			}
			else
			{
				OutHandle[i] = pair;
			}
		}
	}

	void Offset(vector<SDescriptorPair>& OutHandle)
	{
		for (auto& handle : OutHandle)
		{
			Offset(handle.CPUHandle, handle.GPUHandle, handle.Index);
		}
	}

	SDescriptorPair Offset(const uint32_t Value = 1)
	{
		const auto oldCPU = CPUHandle;
		const auto oldGPU = GPUHandle;
		const auto oldOffset = CurrentOffset;
		CurrentOffset += Value;
		CPUHandle.Offset(Value, DescSize);
		if (GPUHandle.ptr != 0)
		{
			GPUHandle.Offset(Value, DescSize);
		}
		Check();
		return { oldCPU, oldGPU, oldOffset };
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

struct SRenderObjectHeap : IDescriptor
{
	void Init(UINT SRVCBVUAVDescSize, UINT RTVDescSize, UINT DSVDescSize);

	TDescriptorHandle SRVHandle;
	ComPtr<ID3D12DescriptorHeap> SRVHeap = nullptr;

	TDescriptorHandle RTVHandle;
	ComPtr<ID3D12DescriptorHeap> RTVHeap = nullptr;

	TDescriptorHandle DSVHandle;
	ComPtr<ID3D12DescriptorHeap> DSVHeap = nullptr;
};

inline wstring ToString(const ComPtr<ID3D12DescriptorHeap>& Heap)
{
	if (Heap)
	{
		wstring text = TEXT(Heap->GetDesc().Type);
		text += L" ";
		text += to_wstring(Heap->GetDesc().NumDescriptors);
		return text;
	}
	return L"nullptr";
}

template<typename DataType>
struct TUploadBufferData
{
	int32_t StartIndex = -1;
	int32_t EndIndex = INT32_MAX;
	OUploadBuffer<DataType>* Buffer = nullptr;
	void PutData(const DataType& Data)
	{
		Buffer->CopyData(StartIndex, Data);
	}
};

enum EResourceHeapType : uint32_t
{
	Default = 1 << 0,
	Shadow = 1 << 1
};

inline wstring ToString(EResourceHeapType Type)
{
	switch (Type)
	{
	case EResourceHeapType::Default:
		return L"Default";
	case EResourceHeapType::Shadow:
		return L"Shadow";
	default:
		return L"Unknown";
	}
}

using SResourceHeapBitFlag = uint32_t;

/**
 * @brief Object being render to the screen, may demand SRV, RTV, DSV, and pass constants
 */
class IRenderObject
{
public:
	virtual ~IRenderObject() = default;
	virtual void BuildDescriptors(IDescriptor* Descriptor){};
	virtual void InitRenderObject(){};
	virtual uint32_t GetNumSRVRequired() const { return 0; }
	virtual uint32_t GetNumRTVRequired() const { return 0; }
	virtual uint32_t GetNumDSVRequired() const { return 0; }
	virtual uint32_t GetNumPassesRequired() const { return 0; }
	virtual void UpdatePass(const TUploadBufferData<SPassConstants>& Data) {}
	virtual void Update(const UpdateEventArgs& Event) {}
	virtual TUUID GetID() { return {}; }
	virtual void SetID(TUUID ID) {}
	virtual wstring GetName() { return { L"UNIMPLEMENTED" }; }
	virtual EResourceHeapType GetHeapType() { return EResourceHeapType::Default; }
};

class ORenderObjectBase : public IRenderObject
{
public:
	ORenderObjectBase() = default;
	explicit ORenderObjectBase(EResourceHeapType InHeapType)
	    : HeapType(InHeapType) {}

	void SetID(TUUID ID) override
	{
		this->ID = ID;
	}
	TUUID GetID() override
	{
		return ID;
	}
	EResourceHeapType GetHeapType() override { return HeapType; }

protected:
	TUUID ID = {};
	EResourceHeapType HeapType = EResourceHeapType::Default;
};