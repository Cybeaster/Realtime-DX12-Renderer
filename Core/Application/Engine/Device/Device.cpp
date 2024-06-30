#pragma once

#include "Device.h"

#include "DirectX/Resource.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Logger.h"

bool ODevice::Init()
{
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	THROW_IF_FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&Factory)));
	if (!DirectX::XMVerifyCPUSupport())
	{
		WIN_LOG(Default, Error, "Failed to verify DirectX Math library support.");
		return false;
	}
	if (const auto adapter = GetAdapter(false))
	{
		Device = CreateDevice(adapter);
	}
	return true;
}

ComPtr<IDXGIAdapter4> ODevice::GetAdapter(bool UseWarp) const
{
	ComPtr<IDXGIAdapter1> dxgiAdapter1;
	ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (UseWarp)
	{
		THROW_IF_FAILED(Factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		THROW_IF_FAILED(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else
	{
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; Factory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				THROW_IF_FAILED(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

ID3D12Device5* ODevice::GetDevice() const
{
	return Device.Get();
}

IDXGIFactory4* ODevice::GetFactory() const
{
	return Factory.Get();
}

void ODevice::CreateShaderResourceView(const TResourceInfo& Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair, bool EmptyResource) const
{
	DescriptorPair.Resource = Resource;
	const auto res = EmptyResource ? nullptr : Resource->Resource.Get();
	Device->CreateShaderResourceView(res, &Desc, DescriptorPair.CPUHandle);
}

void ODevice::CreateUnorderedAccessView(const TResourceInfo& Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const
{
	DescriptorPair.Resource = Resource;
	Device->CreateUnorderedAccessView(Resource->Resource.Get(), nullptr, &Desc, DescriptorPair.CPUHandle);
}

void ODevice::CreateRenderTargetView(const TResourceInfo& Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const
{
	DescriptorPair.Resource = Resource;
	Device->CreateRenderTargetView(Resource->Resource.Get(), &Desc, DescriptorPair.CPUHandle);
}

void ODevice::CreateRenderTargetView(const TResourceInfo& Resource, SDescriptorPair& DescriptorPair) const
{
	DescriptorPair.Resource = Resource;
	Device->CreateRenderTargetView(Resource->Resource.Get(), nullptr, DescriptorPair.CPUHandle);
}

void ODevice::CreateDepthStencilView(const TResourceInfo& Resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const
{
	DescriptorPair.Resource = Resource;
	Device->CreateDepthStencilView(Resource->Resource.Get(), &Desc, DescriptorPair.CPUHandle);
}

void ODevice::CreateDepthStencilView(const TResourceInfo& Resource, SDescriptorPair& DescriptorPair) const
{
	DescriptorPair.Resource = Resource;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = SRenderConstants::DepthBufferDSVFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(Resource->Resource.Get(), &dsvDesc, DescriptorPair.CPUHandle);
}

HRESULT ODevice::CheckDeviceRemoveReason() const
{
	auto removeReason = Device->GetDeviceRemovedReason();
	if (SUCCEEDED(Device->GetDeviceRemovedReason()))
	{
		return S_OK;
	}

	THROW(removeReason);
	return E_FAIL;
}

ComPtr<ID3D12Device5> ODevice::CreateDevice(const ComPtr<IDXGIAdapter4>& Adapter)
{
	ComPtr<ID3D12Device5> device5;
	THROW_IF_FAILED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device5)));
	//    NAME_D3D12_OBJECT(d3d12Device2);

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(device5.As(&pInfoQueue)))
	{
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		// D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE, // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		// NewFilter.DenyList.NumCategories = _countof(Categories);
		// NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		THROW_IF_FAILED(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return device5;
}