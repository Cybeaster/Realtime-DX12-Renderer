

#include "BilateralBlurFilter.h"

#include "DirectX/ShaderTypes.h"

OBilateralBlurFilter::OBilateralBlurFilter(ID3D12Device* Device, ID3D12GraphicsCommandList* List, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, List, Width, Height, Format)
{
	BlurBuffer = OUploadBuffer<SBilateralBlur>::Create(Device, 1, true, this);
	BufferConstants = OUploadBuffer<SBufferConstants>::Create(Device, 1, true, this);
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

void OBilateralBlurFilter::OutputTo(SResourceInfo* Destination) const
{
	Utils::ResourceBarrier(CMDList, Destination, D3D12_RESOURCE_STATE_COPY_DEST);
	CMDList->CopyResource(Destination->Resource.Get(), OutputTexture.Resource.Get());
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

	Device->CreateShaderResourceView(OutputTexture.Resource.Get(), &srvDesc, BlurOutputSrvHandle.CPUHandle);
	Device->CreateUnorderedAccessView(OutputTexture.Resource.Get(), nullptr, &uavDesc, BlurOutputUavHandle.CPUHandle);

	Device->CreateShaderResourceView(InputTexture.Resource.Get(), &srvDesc, BlurInputSrvHandle.CPUHandle);
	Device->CreateUnorderedAccessView(InputTexture.Resource.Get(), nullptr, &uavDesc, BlurInputUavHandle.CPUHandle);
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

	OutputTexture = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
	InputTexture = Utils::CreateResource(this, Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
}

void OBilateralBlurFilter::Execute(const SPSODescriptionBase* PSO, SResourceInfo* Input)
{
	using namespace Utils;
	PSO->RootSignature->ActivateRootSignature(CMDList);
	CMDList->SetPipelineState(PSO->PSO.Get());
	BlurBuffer->CopyData(0, { SpatialSigma, IntensitySigma, BlurCount });
	BufferConstants->CopyData(0, { Width, Height });
	PSO->RootSignature->SetResourceCBView("BilateralBlur", BlurBuffer->GetGPUAddress(), CMDList);
	PSO->RootSignature->SetResourceCBView("BufferConstants", BufferConstants->GetGPUAddress(), CMDList);

	ResourceBarrier(CMDList, Input, D3D12_RESOURCE_STATE_COPY_SOURCE);
	ResourceBarrier(CMDList, &InputTexture, D3D12_RESOURCE_STATE_COPY_DEST);

	CMDList->CopyResource(InputTexture.Resource.Get(), Input->Resource.Get());

	ResourceBarrier(CMDList, &InputTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(CMDList, &OutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	PSO->RootSignature->SetDescriptorTable("Input", BlurInputSrvHandle.GPUHandle, CMDList);
	PSO->RootSignature->SetDescriptorTable("Output", BlurOutputUavHandle.GPUHandle, CMDList);

	CMDList->Dispatch(Width / 32 + 1, Height / 32 + 1, 1);

	ResourceBarrier(CMDList, &OutputTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(CMDList, &InputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}
