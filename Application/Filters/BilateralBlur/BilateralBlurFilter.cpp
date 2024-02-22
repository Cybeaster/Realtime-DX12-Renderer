//
// Created by Cybea on 29/01/2024.
//

#include "BilateralBlurFilter.h"

#include "../../../Utils/DirectXUtils.h"
#include "../../../Utils/Statics.h"

OBilateralBlurFilter::OBilateralBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, List, Width, Height, Format)
{
}

void OBilateralBlurFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}

	descriptor->SRVHandle.Offset(BlurOutputSrvHandle);
	descriptor->SRVHandle.Offset(BlurOutputUavHandle);
	descriptor->SRVHandle.Offset(BlurInputSrvHandle);
	descriptor->SRVHandle.Offset(BlurInputUavHandle);

	BuildDescriptors();
}

void OBilateralBlurFilter::OutputTo(ID3D12Resource* Destination) const
{
	Utils::ResourceBarrier(CMDList, Destination, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	CMDList->CopyResource(Destination, OutputTexture.Get());
}

void OBilateralBlurFilter::BuildDescriptors() const
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

	Device->CreateShaderResourceView(OutputTexture.Get(), &srvDesc, BlurOutputSrvHandle.CPUHandle);
	Device->CreateUnorderedAccessView(OutputTexture.Get(), nullptr, &uavDesc, BlurOutputUavHandle.CPUHandle);

	Device->CreateShaderResourceView(InputTexture.Get(), &srvDesc, BlurInputSrvHandle.CPUHandle);
	Device->CreateUnorderedAccessView(InputTexture.Get(), nullptr, &uavDesc, BlurInputUavHandle.CPUHandle);
}

void OBilateralBlurFilter::BuildResource()
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

	const auto commonState = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(Device->CreateCommittedResource(&commonState,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_COMMON,
	                                                nullptr,
	                                                IID_PPV_ARGS(OutputTexture.GetAddressOf())));

	THROW_IF_FAILED(Device->CreateCommittedResource(&commonState,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_COMMON,
	                                                nullptr,
	                                                IID_PPV_ARGS(InputTexture.GetAddressOf())));
}

void OBilateralBlurFilter::Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, ID3D12Resource* Input) const
{
	using namespace Utils;

	CMDList->SetComputeRootSignature(RootSignature);

	CMDList->SetComputeRoot32BitConstants(0, 1, &SpatialSigma, 0);
	CMDList->SetComputeRoot32BitConstants(0, 1, &IntensitySigma, 1);
	CMDList->SetComputeRoot32BitConstants(0, 1, &BlurCount, 2);

	const array<uint32_t, 2> BufferConstants = { Width, Height };
	CMDList->SetComputeRoot32BitConstants(1, 2, BufferConstants.data(), 0);

	ResourceBarrier(CMDList, Input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ResourceBarrier(CMDList, InputTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	CMDList->CopyResource(InputTexture.Get(), Input);

	ResourceBarrier(CMDList, InputTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(CMDList, OutputTexture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	CMDList->SetPipelineState(PSO);

	CMDList->SetComputeRootDescriptorTable(2, BlurInputSrvHandle.GPUHandle);
	CMDList->SetComputeRootDescriptorTable(3, BlurOutputUavHandle.GPUHandle);

	CMDList->Dispatch(Width / 32 + 1, Height / 32 + 1, 1);

	ResourceBarrier(CMDList, OutputTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(CMDList, InputTexture.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}