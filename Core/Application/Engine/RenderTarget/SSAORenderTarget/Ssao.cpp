#include "Ssao.h"

#include "DirectX/DXHelper.h"
#include "DirectXPackedVector.h"
#include "Engine/Engine.h"
#include "Window/Window.h"
using namespace DirectX;
using namespace DirectX::PackedVector;

void OSSAORenderTarget::BuildDescriptors(IDescriptor* Descriptor)
{
	if (auto descriptor = Cast<SRenderObjectHeap>(Descriptor))
	{
		descriptor->SRVHandle.Offset(NormalMapSRV);
		descriptor->RTVHandle.Offset(NormalMapRTV);
		descriptor->SRVHandle.Offset(DepthMapSRV);
		descriptor->SRVHandle.Offset(RandomVectorMapSRV);
		descriptor->SRVHandle.Offset(AmbientMap0SRV);
		descriptor->RTVHandle.Offset(AmbientMap0RTV);
		descriptor->SRVHandle.Offset(AmbientMap1SRV);
		descriptor->RTVHandle.Offset(AmbientMap1RTV);
		BuildDescriptors();
	}
}

void OSSAORenderTarget::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	auto device = Device.lock();
	device->CreateShaderResourceView(NormalMap, srvDesc, NormalMapSRV);

	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	device->CreateShaderResourceView(DepthMap, srvDesc, DepthMapSRV);

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateShaderResourceView(RandomVectorMap, srvDesc, RandomVectorMapSRV);

	srvDesc.Format = SRenderConstants::AmbientMapFormat;
	device->CreateShaderResourceView(AmbientMap0, srvDesc, AmbientMap0SRV);
	device->CreateShaderResourceView(AmbientMap1, srvDesc, AmbientMap1SRV);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	device->CreateRenderTargetView(NormalMap, rtvDesc, NormalMapRTV);

	rtvDesc.Format = SRenderConstants::AmbientMapFormat;
	device->CreateRenderTargetView(AmbientMap0, AmbientMap0RTV);
	device->CreateRenderTargetView(AmbientMap1, AmbientMap1RTV);
}

void OSSAORenderTarget::BuildResource()
{
	// Free the old resources if they exist.
	NormalMap = nullptr;
	AmbientMap0 = nullptr;
	AmbientMap1 = nullptr;

	auto desc = GetResourceDesc();
	auto device = Device.lock();
	float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	CD3DX12_CLEAR_VALUE optClear(Format, normalClearColor);
	auto weak = weak_from_this();
	NormalMap = Utils::CreateResource(weak, L"NormalMap", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear);

	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	DepthMap = Utils::CreateResource(weak, L"DepthMap", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_RESOURCE_STATE_GENERIC_READ);

	desc.Width = Width / 2;
	desc.Height = Height / 2;
	desc.Format = SRenderConstants::AmbientMapFormat;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	float ambientClearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	optClear = CD3DX12_CLEAR_VALUE(SRenderConstants::AmbientMapFormat, ambientClearColor);

	AmbientMap0 = Utils::CreateResource(weak, L"AmbientMap0", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear);
	AmbientMap1 = Utils::CreateResource(weak, L"AmbientMap1", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear);
}

void OSSAORenderTarget::BuildOffsetVectors()
{
	using namespace DirectX;
	// 8 cube corners
	Offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	Offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	Offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	Offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	Offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	Offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	Offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	Offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	Offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	Offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	Offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	Offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	Offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	Offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		float s = Utils::Math::Random(0.25f, 1.0f);
		XMVECTOR v = XMLoadFloat4(&Offsets[i]);

		v = XMVectorScale(XMVector4Normalize(v), s);
		XMStoreFloat4(&Offsets[i], v);
	}
}

void OSSAORenderTarget::BuildRandomVectorTexture()
{
	auto desc = GetResourceDesc();
	desc.Width = 256;
	desc.Height = 256;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	auto device = Device.lock();
	auto weak = weak_from_this();
	RandomVectorMap = Utils::CreateResource(weak, L"RandomVectorMap", device->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, desc, D3D12_RESOURCE_STATE_GENERIC_READ);
	const UINT num2DSubresources = desc.DepthOrArraySize * desc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(RandomVectorMap->Resource.Get(), 0, num2DSubresources);
	RandomVectorMapUploadBuffer = Utils::CreateResource(weak, L"RandomVectorMapUploadBuffer", device->GetDevice(), D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ);

	XMCOLOR initData[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			XMFLOAT3 v(Utils::Math::Random<float>(), Utils::Math::Random<float>(), Utils::Math::Random<float>());
			initData[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * 256;

	auto queue = OEngine::Get()->GetCommandQueue();
	queue->TryResetCommandList();
	queue->ResourceBarrier(RandomVectorMap.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	UpdateSubresources(queue->GetCommandList().Get(), RandomVectorMap->Resource.Get(), RandomVectorMapUploadBuffer->Resource.Get(), 0, 0, num2DSubresources, &subResourceData);
	queue->ResourceBarrier(RandomVectorMap.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	queue->ExecuteCommandList();
}

void OSSAORenderTarget::InitRenderObject()
{
	auto weak = weak_from_this();
	ORenderTargetBase::InitRenderObject();
	HBlurCB = make_unique<OUploadBuffer<SConstantBufferBlurData>>(Device, 1, true, weak);
	HBlurCB->CopyData(0, SConstantBufferBlurData{ 0 });
	VBlurCB = make_unique<OUploadBuffer<SConstantBufferBlurData>>(Device, 1, true, weak);
	VBlurCB->CopyData(0, SConstantBufferBlurData{ 1 });
	BuildResource();
	BuildOffsetVectors();
	BuildRandomVectorTexture();
}

void OSSAORenderTarget::BuildViewport()
{
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.Width = static_cast<float>(Width / 2);
	Viewport.Height = static_cast<float>(Height / 2);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, Width / 2, Height / 2 };
}

void OSSAORenderTarget::OnResize(const ResizeEventArgs& Args)
{
	if (Width != Args.Width || Height != Args.Height)
	{
		Width = Args.Width;
		Height = Args.Height;
		BuildViewport();
		BuildResource();
	}
	BuildDescriptors();
}

std::array<DirectX::XMFLOAT4, 14> OSSAORenderTarget::GetOffsetVectors()
{
	return Offsets;
}

SDescriptorPair OSSAORenderTarget::GetAmbientMap0SRV()
{
	return AmbientMap0SRV;
}

SDescriptorPair OSSAORenderTarget::GetAmbientMap1SRV()
{
	return AmbientMap1SRV;
}

vector<float> OSSAORenderTarget::CalcGaussWeights() const
{
	float twoSigma2 = 2.0f * GauseWeightSigma * GauseWeightSigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is
	int blurRadius = (int)ceil(2.0f * GauseWeightSigma);

	if (blurRadius > MaxBlurRadius)
	{
		LOG(Render, Warning, "Blur radius is too large.")
		blurRadius = MaxBlurRadius;
	}

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		auto x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (float& weight : weights)
	{
		weight /= weightSum;
	}

	return weights;
}

SResourceInfo* OSSAORenderTarget::GetSubresource(uint32_t Idx)
{
	if (Idx == NormalSubtarget)
	{
		return NormalMap.get();
	}
	else if (Idx == AmbientSubtarget0)
	{
		return AmbientMap0.get();
	}
	else if (Idx == AmbientSubtarget1)
	{
		return AmbientMap1.get();
	}
	return nullptr;
}

uint32_t OSSAORenderTarget::GetNumRTVRequired() const
{
	return 3;
}

uint32_t OSSAORenderTarget::GetNumSRVRequired() const
{
	return 5;
}

SResourceInfo* OSSAORenderTarget::GetRandomVectorMap()
{
	return RandomVectorMap.get();
}

SResourceInfo* OSSAORenderTarget::GetNormalMap()
{
	return NormalMap.get();
}

SDescriptorPair OSSAORenderTarget::GetNormalMapSRV() const
{
	return NormalMapSRV;
}

SDescriptorPair OSSAORenderTarget::GetNormalMapRTV() const
{
	return NormalMapRTV;
}

SDescriptorPair OSSAORenderTarget::GetDepthMapSRV() const
{
	return DepthMapSRV;
}

SDescriptorPair OSSAORenderTarget::GetRandomVectorMapSRV() const
{
	return RandomVectorMapSRV;
}

SDescriptorPair OSSAORenderTarget::GetAmbientMap0RTV() const
{
	return AmbientMap0RTV;
}

SDescriptorPair OSSAORenderTarget::GetAmbientMap1RTV() const
{
	return AmbientMap1RTV;
}

SResourceInfo* OSSAORenderTarget::GetAmbientMap0()
{
	return AmbientMap0.get();
}

SResourceInfo* OSSAORenderTarget::GetDepthMap()
{
	return DepthMap.get();
}

void OSSAORenderTarget::PrepareRenderTarget(OCommandQueue* Queue, bool ClearRenderTarget, bool ClearDepth, uint32_t SubtargetIdx)
{
	if (SubtargetIdx == NormalSubtarget)
	{
		Queue->ClearRenderTarget(NormalMapRTV, SColor::Blue);
		Queue->ClearDepthStencil(GetDSV());
		Queue->SetRenderTargets(NormalMapRTV, GetDSV());
		LOG(Engine, Log, "Setting render target with address: [{}] in [{}] and depth stencil: [{}]", TEXT(NormalMapRTV.Index), GetName(), TEXT(GetDSV().Index));
	}
	else if (SubtargetIdx == AmbientSubtarget0)
	{
		Queue->SetViewportScissors(Viewport, ScissorRect);
		float clearValue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		Queue->ClearRenderTarget(AmbientMap0RTV, SColor::White);
		Queue->SetRenderToRTVOnly(AmbientMap0RTV);
		LOG(Engine, Log, "Setting render target with address: [{}] in [{}]and depth stencil: null", TEXT(AmbientMap0RTV.CPUHandle.ptr), GetName());
	}
}

D3D12_GPU_VIRTUAL_ADDRESS OSSAORenderTarget::GetHBlurCBAddress() const
{
	return HBlurCB->GetGPUAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS OSSAORenderTarget::GetVBlurCBAddress() const
{
	return VBlurCB->GetGPUAddress();
}

SResourceInfo* OSSAORenderTarget::GetResource()
{
	return nullptr;
}
