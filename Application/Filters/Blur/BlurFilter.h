#pragma once
#include "Filters/FilterBase.h"

#include <Types.h>
#include <d3dx12.h>
#include <wrl/client.h>
class OBlurFilter : public OFilterBase
{
public:
	OBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format);
	OBlurFilter(const OBlurFilter& rhs) = delete;
	OBlurFilter& operator=(const OBlurFilter& rhs) = delete;

	void OutputTo(ID3D12Resource* Destination) const;

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void Execute(
	    ID3D12RootSignature* RootSignature,
	    ID3D12PipelineState* HorizontalBlurPSO,
	    ID3D12PipelineState* VerticalBlurPSO,
	    ID3D12Resource* Input,
	    int BlurCount) const;
	uint32_t GetNumDescriptors() const override
	{
		return 4;
	}

private:
	vector<float> CalcGaussWeights(float Sigma) const;

	void BuildDescriptors() const override;
	void BuildResource() override;

private:
	const uint32_t MaxBlurRadius = 5;

	CD3DX12_CPU_DESCRIPTOR_HANDLE Blur0CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Blur0CpuUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE Blur1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Blur1CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE Blur0GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE Blur0GpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE Blur1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE Blur1GpuUav;

	Microsoft::WRL::ComPtr<ID3D12Resource> BlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> BlurMap1 = nullptr;
};
