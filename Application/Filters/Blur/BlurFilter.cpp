#include "BlurFilter.h"

#include "../../../Utils/Statics.h"
#include "../../Utils/DirectXUtils.h"
#include "Logger.h"

OBlurFilter::OBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, List, Width, Height, Format)
{
}

void OBlurFilter::OutputTo(ID3D12Resource* Destination) const
{
	Utils::ResourceBarrier(CMDList, Destination, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	CMDList->CopyResource(Destination, BlurMap0.Get());
}

void OBlurFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}

	descriptor->SRVHandle.Offset(SRV0Handle);
	descriptor->SRVHandle.Offset(UAV0Handle);
	descriptor->SRVHandle.Offset(SRV1Handle);
	descriptor->SRVHandle.Offset(UAV1Handle);

	BuildDescriptors();
}

void OBlurFilter::BuildDescriptors() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	Device->CreateShaderResourceView(BlurMap0.Get(), &srvDesc, SRV0Handle.CPUHandle);
	Device->CreateUnorderedAccessView(BlurMap0.Get(), nullptr, &uavDesc, UAV0Handle.CPUHandle);

	Device->CreateShaderResourceView(BlurMap1.Get(), &srvDesc, SRV1Handle.CPUHandle);
	Device->CreateUnorderedAccessView(BlurMap1.Get(), nullptr, &uavDesc, UAV1Handle.CPUHandle);
}

void OBlurFilter::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc = {};
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));

	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = Width;
	texDesc.Height = Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	auto commonState = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(Device->CreateCommittedResource(&commonState,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_COMMON,
	                                                nullptr,
	                                                IID_PPV_ARGS(BlurMap0.GetAddressOf())));

	THROW_IF_FAILED(Device->CreateCommittedResource(&commonState,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_COMMON,
	                                                nullptr,
	                                                IID_PPV_ARGS(BlurMap1.GetAddressOf())));
}

void OBlurFilter::Execute(
    ID3D12RootSignature* RootSignature,
    ID3D12PipelineState* HorizontalBlurPSO,
    ID3D12PipelineState* VerticalBlurPSO,
    ID3D12Resource* Input) const
{
	using namespace Utils;
	const auto weights = CalcGaussWeights(Sigma);
	const auto blurRadius = static_cast<int32_t>(weights.size() / 2);

	CMDList->SetComputeRootSignature(RootSignature);

	CMDList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	CMDList->SetComputeRoot32BitConstants(0, static_cast<UINT>(weights.size()), weights.data(), 1);

	ResourceBarrier(CMDList, Input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ResourceBarrier(CMDList, BlurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	CMDList->CopyResource(BlurMap0.Get(), Input);

	ResourceBarrier(CMDList, BlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(CMDList, BlurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	for (int i = 0; i < BlurCount; i++)
	{
		//Horizontal blur
		CMDList->SetPipelineState(HorizontalBlurPSO);
		CMDList->SetComputeRootDescriptorTable(1, SRV0Handle.GPUHandle);
		CMDList->SetComputeRootDescriptorTable(2, UAV1Handle.GPUHandle);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).

		const UINT numGroupsX = (UINT)ceilf(Width / 256.0f);
		CMDList->Dispatch(numGroupsX, Height, 1);

		ResourceBarrier(CMDList, BlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		ResourceBarrier(CMDList, BlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

		// vertical BLur
		CMDList->SetPipelineState(VerticalBlurPSO);
		CMDList->SetComputeRootDescriptorTable(1, SRV1Handle.GPUHandle);
		CMDList->SetComputeRootDescriptorTable(2, UAV0Handle.GPUHandle);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).

		UINT numGroupsY = (UINT)ceilf(Height / 256.0f);
		CMDList->Dispatch(Width, numGroupsY, 1);

		ResourceBarrier(CMDList, BlurMap0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		ResourceBarrier(CMDList, BlurMap1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

vector<float> OBlurFilter::CalcGaussWeights(float Sigma) const
{
	float twoSigma = 2.0f * Sigma * Sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is
	int blurRadius = static_cast<int>(ceil(2.0f * Sigma));

	if (blurRadius > MaxBlurRadius)
	{
		LOG(Engine, Warning, "Blur radius is too big: {}, Max blur radius is {}.", blurRadius, MaxBlurRadius);
		blurRadius = MaxBlurRadius;
	}

	vector<float> weights;
	weights.resize(2 * blurRadius + 1);
	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = static_cast<float>(i);
		weights[i + blurRadius] = expf(-x * x / twoSigma);
		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (auto& w : weights)
	{
		w /= weightSum;
	}
	return weights;
}
