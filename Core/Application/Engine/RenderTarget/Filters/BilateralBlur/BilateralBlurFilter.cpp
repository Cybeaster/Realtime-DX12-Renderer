

#include "BilateralBlurFilter.h"

#include "DirectX/ShaderTypes.h"

OBilateralBlurFilter::OBilateralBlurFilter(const weak_ptr<ODevice>& Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, Other, Width, Height, Format)
{
}
void OBilateralBlurFilter::InitRenderObject()
{
	OFilterBase::InitRenderObject();
	auto weak = weak_from_this();

	BlurBuffer = OUploadBuffer<SBilateralBlur>::Create(Device, 1, true, weak);
	BufferConstants = OUploadBuffer<SBufferConstants>::Create(Device, 1, true, weak);
	FilterName = L"BilateralBlurFilter";
}

void OBilateralBlurFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectHeap>(Descriptor);
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

void OBilateralBlurFilter::OutputTo(SResourceInfo* Destination)
{
	Queue->CopyResourceTo(Destination, OutputTexture.get());
}

void OBilateralBlurFilter::BuildDescriptors()
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
	auto device = Device.lock();
	device->CreateShaderResourceView(OutputTexture, srvDesc, BlurOutputSrvHandle);
	device->CreateUnorderedAccessView(OutputTexture, uavDesc, BlurOutputUavHandle);

	device->CreateShaderResourceView(InputTexture, srvDesc, BlurInputSrvHandle);
	device->CreateUnorderedAccessView(InputTexture, uavDesc, BlurInputUavHandle);
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
	auto device = Device.lock()->GetDevice();
	auto weak = weak_from_this();
	OutputTexture = Utils::CreateResource(weak, L"OutupTexture", device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
	InputTexture = Utils::CreateResource(weak, L"InputTexture", device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
}

void OBilateralBlurFilter::Execute(const SPSODescriptionBase* PSO, SResourceInfo* Input)
{
	using namespace Utils;
	PSO->RootSignature->ActivateRootSignature(Queue->GetCommandList().Get());
	auto cmd = Queue->GetCommandList().Get();
	BlurBuffer->CopyData(0, { SpatialSigma, IntensitySigma, BlurCount });
	BufferConstants->CopyData(0, { Width, Height });
	PSO->RootSignature->SetResource("BilateralBlur", BlurBuffer->GetGPUAddress(), cmd);
	PSO->RootSignature->SetResource("BufferConstants", BufferConstants->GetGPUAddress(), cmd);

	Queue->CopyResourceTo(InputTexture.get(), Input);

	ResourceBarrier(cmd, InputTexture.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(cmd, OutputTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	PSO->RootSignature->SetResource("Input", BlurInputSrvHandle.GPUHandle, cmd);
	PSO->RootSignature->SetResource("Output", BlurOutputUavHandle.GPUHandle, cmd);

	cmd->Dispatch(Width / 32 + 1, Height / 32 + 1, 1);

	ResourceBarrier(cmd, OutputTexture.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(cmd, InputTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}
