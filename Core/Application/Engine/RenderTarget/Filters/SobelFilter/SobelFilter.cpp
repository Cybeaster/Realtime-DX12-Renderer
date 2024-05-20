#include "SobelFilter.h"

#include "DirectX/ShaderTypes.h"

OSobelFilter::OSobelFilter(const weak_ptr<ODevice>& Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, Other, Width, Height, Format)
{
	FilterName = L"SobelFilter";
}

CD3DX12_GPU_DESCRIPTOR_HANDLE OSobelFilter::OutputSRV() const
{
	return OutputSRVHandle.GPUHandle;
}

void OSobelFilter::BuildDescriptors()
{
	auto device = Device.lock();
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

	device->CreateShaderResourceView(Output, srvDesc, OutputSRVHandle);
	device->CreateUnorderedAccessView(Output, uavDesc, OutputUAVHandle);
	device->CreateShaderResourceView(Input, srvDesc, InputSRVHandle);
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
	auto device = Device.lock()->GetDevice();
	auto weak = weak_from_this();
	Output = Utils::CreateResource(weak, L"Output", device, D3D12_HEAP_TYPE_DEFAULT, texDesc);
	Input = Utils::CreateResource(weak, L"Input", device, D3D12_HEAP_TYPE_DEFAULT, texDesc);
}

bool OSobelFilter::Execute(SPSODescriptionBase* PSO, ORenderTargetBase* InTarget)
{
	if (!bEnabled)
	{
		return false;
	}

	Queue->CopyResourceTo(Input.get(), InTarget->GetResource());
	auto cmd = Queue->GetCommandList().Get();
	Queue->SetPipelineState(PSO);

	auto root = PSO->RootSignature;
	root->SetResource("Input", InputSRVHandle.GPUHandle, cmd);
	root->SetResource("Output", OutputUAVHandle.GPUHandle, cmd);

	Utils::ResourceBarrier(cmd, Output.get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	UINT numGroupsX = (UINT)ceilf(Width / 16.0f);
	UINT numGroupsY = (UINT)ceilf(Height / 16.0f);
	cmd->Dispatch(numGroupsX, numGroupsY, 1);

	Utils::ResourceBarrier(cmd, Output.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
	return true;
}

void OSobelFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	auto descriptor = Cast<SRenderObjectHeap>(Descriptor);
	if (!descriptor)
	{
		return;
	}
	descriptor->SRVHandle.Offset(OutputSRVHandle);
	descriptor->SRVHandle.Offset(OutputUAVHandle);
	descriptor->SRVHandle.Offset(InputSRVHandle);

	BuildDescriptors();
}

bool OSobelFilter::IsEnabled() const
{
	return bEnabled;
}

bool OSobelFilter::IsPureSobel() const
{
	return PureSobel;
}
SResourceInfo* OSobelFilter::GetOutput() const
{
	return Output.get();
}