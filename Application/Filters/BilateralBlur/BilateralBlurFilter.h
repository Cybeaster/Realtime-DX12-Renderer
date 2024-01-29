#pragma once
#include "Filters/FilterBase.h"

class OBilateralBlurFilter : public OFilterBase
{
public:
	OBilateralBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE HCPUDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE HGPUDescriptor, UINT DescriptorSize) override;

	void OutputTo(ID3D12Resource* Destination) const;
	void BuildDescriptors() const override;
	void BuildResource() override;
	void Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO,
	             ID3D12Resource* Input, float SpatialSigma, float IntensitySigma, int32_t BlurCount) const;

private:
	CD3DX12_CPU_DESCRIPTOR_HANDLE BlurOutputCpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE BlurOutputCpuUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE BlurInputCpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE BlurInputCpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE BlurOutputGpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE BlurOutputGpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE BlurInputGpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE BlurInputGpuUav;

	ComPtr<ID3D12Resource> InputTexture;
	ComPtr<ID3D12Resource> OutputTexture;
};
