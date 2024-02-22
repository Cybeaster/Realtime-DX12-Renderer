#include "SobelFilter.h"

#include "../../../Utils/DirectXUtils.h"
#include "../../../Utils/Statics.h"
#include "RenderConstants.h"

OSobelFilter::OSobelFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, List, Width, Height, Format)
{
}

CD3DX12_GPU_DESCRIPTOR_HANDLE OSobelFilter::OutputSRV() const
{
	return SRVHandle.GPUHandle;
}

void OSobelFilter::BuildDescriptors() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	Device->CreateShaderResourceView(Output.Get(), &srvDesc, SRVHandle.CPUHandle);
	Device->CreateUnorderedAccessView(Output.Get(), nullptr, &uavDesc, UAVHandle.CPUHandle);
}

void OSobelFilter::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
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

	const auto defaultHead = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(Device->CreateCommittedResource(&defaultHead,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &texDesc,
	                                                D3D12_RESOURCE_STATE_GENERIC_READ,
	                                                nullptr,
	                                                IID_PPV_ARGS(Output.GetAddressOf())));
}

std::pair<bool, CD3DX12_GPU_DESCRIPTOR_HANDLE> OSobelFilter::Execute(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, CD3DX12_GPU_DESCRIPTOR_HANDLE Input) const
{
	if (!bEnabled)
	{
		return { false, CD3DX12_GPU_DESCRIPTOR_HANDLE() };
	}

	CMDList->SetComputeRootSignature(RootSignature);
	CMDList->SetPipelineState(PSO);

	CMDList->SetComputeRootDescriptorTable(0, Input);
	CMDList->SetComputeRootDescriptorTable(2, UAVHandle.GPUHandle);

	Utils::ResourceBarrier(CMDList, Output.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	UINT numGroupsX = (UINT)ceilf(Width / 16.0f);
	UINT numGroupsY = (UINT)ceilf(Height / 16.0f);
	CMDList->Dispatch(numGroupsX, numGroupsY, 1);

	Utils::ResourceBarrier(CMDList, Output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	return { true, SRVHandle.GPUHandle };
}

void OSobelFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	auto descriptor = Cast<SRenderObjectDescriptor>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	descriptor->SRVHandle.Offset(SRVHandle);
	descriptor->SRVHandle.Offset(UAVHandle);

	BuildDescriptors();
}