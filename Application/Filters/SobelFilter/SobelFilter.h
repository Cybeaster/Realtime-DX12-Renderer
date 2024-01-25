#pragma once
#include "DXHelper.h"
#include "Filters/FilterBase.h"
class OSobelFilter : public OFilterBase
{
public:
	OSobelFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);
	OSobelFilter(const OSobelFilter& rhs) = delete;
	OSobelFilter& operator=(const OSobelFilter& rhs) = delete;

	CD3DX12_GPU_DESCRIPTOR_HANDLE OutputSRV() const;
	UINT GetDescriptorCount() const;

	void BuildDescriptors() const override;
	void BuildResource() override;

	void Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, CD3DX12_GPU_DESCRIPTOR_HANDLE Input) const;
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE HCPUDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE HGPUDescriptor, UINT DescriptorSize) override;

private:
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUSRV;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUUAV;

	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUSRV;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GPUUAV;

	ComPtr<ID3D12Resource> Output = nullptr;
};
