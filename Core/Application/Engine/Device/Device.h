#pragma once
#include "DirectX/DXHelper.h"
class ODevice
{
public:
	bool Init();
	[[nodiscard]] ID3D12Device5* GetDevice() const;
	[[nodiscard]] IDXGIFactory4* GetFactory() const;

private:
	ComPtr<ID3D12Device5> CreateDevice(const Microsoft::WRL::ComPtr<IDXGIAdapter4>& Adapter);
	ComPtr<IDXGIAdapter4> GetAdapter(bool UseWarp) const;

	ComPtr<IDXGIAdapter4> Adapter;
	ComPtr<IDXGIFactory4> Factory;
	ComPtr<ID3D12Device5> Device;
};
