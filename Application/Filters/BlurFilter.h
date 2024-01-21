#pragma once
#include <Types.h>
#include <d3dx12.h>
#include <wrl/client.h>
class OBlurFilter
{
public:
	OBlurFilter(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format);
	OBlurFilter(const OBlurFilter& rhs) = delete;
	OBlurFilter& operator=(const OBlurFilter& rhs) = delete;
	~OBlurFilter() = default;

	ID3D12Resource* Output() const;

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE HCPUDescriptor, CD3DX12_GPU_DESCRIPTOR_HANDLE HGPUDescriptor, UINT DescriptorSize);
	void OnResize(UINT NewWidth, UINT NewHeight);
	void Execute(ID3D12GraphicsCommandList* CMDList,
	             ID3D12RootSignature* RootSignature,
	             ID3D12PipelineState* HorizontalBlurPSO,
	             ID3D12PipelineState* VerticalBlurPSO,
	             ID3D12Resource* Input,
	             int BlurCount);

private:
	vector<float> CalcGaussWeights(float Sigma) const;

	void BuildDescriptors() const;
	void BuildResources();

private:
	const uint32_t MaxBlurRadius = 5;

	ID3D12Device* Device = nullptr;

	uint32_t Width = 0;
	uint32_t Height = 0;
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

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
