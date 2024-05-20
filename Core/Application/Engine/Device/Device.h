#pragma once
#include "DirectX/DXHelper.h"
#include "DirectX/Resource.h"

struct SDescriptorPair;
struct SResourceInfo;
class ODevice
{
public:
	bool Init();
	[[nodiscard]] ID3D12Device5* GetDevice() const;
	[[nodiscard]] IDXGIFactory4* GetFactory() const;

	void CreateShaderResourceView(const TResourceInfo& Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const;
	void CreateUnorderedAccessView(const TResourceInfo& Resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const;
	void CreateRenderTargetView(const TResourceInfo& Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const;
	void CreateRenderTargetView(const TResourceInfo& Resource, SDescriptorPair& DescriptorPair) const;

	void CreateDepthStencilView(const TResourceInfo& Resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, SDescriptorPair& DescriptorPair) const;
	void CreateDepthStencilView(const TResourceInfo& Resource, SDescriptorPair& DescriptorPair) const;

private:
	ComPtr<ID3D12Device5> CreateDevice(const Microsoft::WRL::ComPtr<IDXGIAdapter4>& Adapter);
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp) const;

	ComPtr<IDXGIAdapter4> Adapter;
	ComPtr<IDXGIFactory4> Factory;
	ComPtr<ID3D12Device5> Device;
};
