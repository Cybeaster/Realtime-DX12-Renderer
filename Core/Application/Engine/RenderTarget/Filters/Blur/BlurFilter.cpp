#include "BlurFilter.h"

#include "DirectX/ShaderTypes.h"
#include "Engine/Device/Device.h"
#include "Logger.h"

OGaussianBlurFilter::OGaussianBlurFilter(const shared_ptr<ODevice>& Device, OCommandQueue* Other, UINT Width, UINT Height, DXGI_FORMAT Format)
    : OFilterBase(Device, Other, Width, Height, Format)
{
}

void OGaussianBlurFilter::InitRenderObject()
{
	OFilterBase::InitRenderObject();
	Buffer = OUploadBuffer<SConstantBlurSettings>::Create(Device, 1, true, weak_from_this());
	FilterName = L"BlurFilter";
}
void OGaussianBlurFilter::OutputTo(SResourceInfo* Destination)
{
	Queue->CopyResourceTo(Destination, BlurMap0.get());
}

void OGaussianBlurFilter::BuildDescriptors(IDescriptor* Descriptor)
{
	const auto descriptor = Cast<SRenderObjectHeap>(Descriptor);
	if (!descriptor)
	{
		return;
	}

	descriptor->SRVHandle.Offset(SRV0Handle);
	descriptor->SRVHandle.Offset(UAV0Handle);
	descriptor->SRVHandle.Offset(SRV1Handle);
	descriptor->SRVHandle.Offset(UAV1Handle);
	descriptor->SRVHandle.Offset(InputSRVHandle);

	BuildDescriptors();
}

void OGaussianBlurFilter::BuildDescriptors()
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

	device->CreateShaderResourceView(BlurMap0, srvDesc, SRV0Handle);
	device->CreateUnorderedAccessView(BlurMap0, uavDesc, UAV0Handle);

	device->CreateShaderResourceView(BlurMap1, srvDesc, SRV1Handle);
	device->CreateUnorderedAccessView(BlurMap1, uavDesc, UAV1Handle);
}

void OGaussianBlurFilter::BuildResource()
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
	auto device = Device.lock();
	auto weak = weak_from_this();
	BlurMap0 = Utils::CreateResource(weak, L"BlurMap0", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
	BlurMap1 = Utils::CreateResource(weak, L"BlurMap1", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);

	InputMap = Utils::CreateResource(weak, L"InputMap", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_COMMON);
}

void OGaussianBlurFilter::Execute(
    const SPSODescriptionBase* HorizontalBlurPSO,
    const SPSODescriptionBase* VerticalBlurPSO,
    SResourceInfo* Input)
{
	using namespace Utils;
	const auto weights = CalcGaussWeights(Sigma);
	const auto blurRadius = static_cast<int32_t>(weights.size() / 2);
	Buffer->CopyData(0,
	                 { blurRadius,
	                   weights[0],
	                   weights[1],
	                   weights[2],
	                   weights[3],
	                   weights[4],
	                   weights[5],
	                   weights[6],
	                   weights[7],
	                   weights[8],
	                   weights[9],
	                   weights[10] });

	auto rootSig = VerticalBlurPSO->RootSignature;
	auto cmdList = Queue->GetCommandList().Get();
	rootSig->ActivateRootSignature(Queue->GetCommandList().Get());
	rootSig->SetResource("cbSettings", Buffer->GetUploadResource()->Resource->GetGPUVirtualAddress(), cmdList);

	Queue->CopyResourceTo(InputMap.get(), Input);
	ResourceBarrier(cmdList, InputMap.get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	ResourceBarrier(cmdList, BlurMap0.get(), D3D12_RESOURCE_STATE_COPY_DEST);

	cmdList->CopyResource(BlurMap0->Resource.Get(), InputMap->Resource.Get());

	ResourceBarrier(cmdList, BlurMap0.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	ResourceBarrier(cmdList, BlurMap1.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	for (int i = 0; i < BlurCount; i++)
	{
		// Horizontal blur
		cmdList->SetPipelineState(HorizontalBlurPSO->PSO.Get());
		rootSig->SetResource("Input", SRV0Handle.GPUHandle, Queue->GetCommandList().Get());
		rootSig->SetResource("Output", UAV1Handle.GPUHandle, Queue->GetCommandList().Get());

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).

		const UINT numGroupsX = (UINT)ceilf(Width / 256.0f);
		Queue->GetCommandList().Get()->Dispatch(numGroupsX, Height, 1);

		ResourceBarrier(Queue->GetCommandList().Get(), BlurMap0.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		ResourceBarrier(Queue->GetCommandList().Get(), BlurMap1.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

		// vertical BLur
		Queue->GetCommandList().Get()->SetPipelineState(VerticalBlurPSO->PSO.Get());
		rootSig->SetResource("Input", SRV1Handle.GPUHandle, Queue->GetCommandList().Get());
		rootSig->SetResource("Output", UAV0Handle.GPUHandle, Queue->GetCommandList().Get());

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).

		UINT numGroupsY = (UINT)ceilf(Height / 256.0f);
		cmdList->Dispatch(Width, numGroupsY, 1);

		ResourceBarrier(cmdList, BlurMap0.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
		ResourceBarrier(cmdList, BlurMap1.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

vector<float> OGaussianBlurFilter::CalcGaussWeights(float Sigma) const
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
	weights.resize(11);
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
