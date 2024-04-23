
#include "ShadowMap.h"

#include "Engine/Engine.h"
#include "LightComponent/LightComponent.h"
void OShadowMap::BuildResource()
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
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	RenderTarget = Utils::CreateResource(this, L"RenderTarget", Device, D3D12_HEAP_TYPE_DEFAULT, texDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &optClear);
}

OShadowMap::OShadowMap(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, OLightComponent* InLightComponent)
    : ORenderTargetBase(Device, Width, Height, Format, EResourceHeapType::Default), LightComponent(InLightComponent)
{
	LightComponent->OnLightChanged.Add([this]() {
		bNeedToUpdate = true;
	});
	Name = L"ShadowMap";
}

void OShadowMap::BuildDescriptors()
{
	// Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	Device->CreateShaderResourceView(RenderTarget.Resource.Get(), &srvDesc, SRV.CPUHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(RenderTarget.Resource.Get(), &dsvDesc, DSV.CPUHandle);
}

void OShadowMap::BuildDescriptors(IDescriptor* Descriptor)
{
	auto desc = Cast<SRenderObjectHeap>(Descriptor);
	if (desc)
	{
		SRV = desc->SRVHandle.Offset();
		DSV = desc->DSVHandle.Offset();
		BuildResource();
		BuildDescriptors();
	}
}

SDescriptorPair OShadowMap::GetSRV(uint32_t SubtargetIdx) const
{
	return SRV;
}

SDescriptorPair OShadowMap::GetDSV(uint32_t SubtargetIdx) const
{
	return DSV;
}

SResourceInfo* OShadowMap::GetResource()
{
	return &RenderTarget;
}

void OShadowMap::PrepareRenderTarget(ID3D12GraphicsCommandList* CommandList, uint32_t SubtargetIdx)
{
	if (PreparedTaregts.contains(SubtargetIdx))
	{
		return;
	}
	PreparedTaregts.insert(SubtargetIdx);
	auto depthStencilView = GetDSV(SubtargetIdx);
	Utils::ResourceBarrier(CommandList, GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	CommandList->ClearDepthStencilView(depthStencilView.CPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(0, nullptr, true, &depthStencilView.CPUHandle);
}

void OShadowMap::UpdatePass(const TUploadBufferData<SPassConstants>& Data)
{
	SetPassConstant();
	Data.Buffer->CopyData(Data.StartIndex, PassConstant);
	PassConstantBuffer = Data;
}

void OShadowMap::SetPassConstant()
{
	using namespace DirectX;
	LightComponent->SetPassConstant(PassConstant);
	PassConstant.RenderTargetSize = XMFLOAT2(static_cast<float>(Width), static_cast<float>(Height));
	PassConstant.InvRenderTargetSize = XMFLOAT2(1.0f / Width, 1.0f / Height);
}

bool OShadowMap::ConsumeUpdate()
{
	if (bNeedToUpdate)
	{
		bNeedToUpdate = false;
		return true;
	}
	return false;
}

void OShadowMap::UpdateLightSourceData()
{
	using namespace DirectX;
	auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const auto& bounds = OEngine::Get()->GetSceneBounds();
	XMVECTOR lightDir;
	XMVECTOR lightTarget; // Calculate target point using direction
	XMMATRIX T(
	    0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
	XMVECTOR lightPos;

	auto lightType = LightComponent->GetLightType();

	if (lightType == ELightType::Directional)
	{
		auto dirLight = LightComponent->GetDirectionalLight();
		lightDir = XMVector3Normalize(XMLoadFloat3(&dirLight.Direction));
		lightPos = -2.0f * bounds.Radius * lightDir;
		lightTarget = Load(bounds.Center); // Calculate target point using direction

		// Assuming lightDir is already normalized
		auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);

		// Transform bounding sphere to light space.
		XMFLOAT3 sphereCenterLS;
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(lightTarget, view));
		// Ortho frustum in light space encloses scene.
		float l = sphereCenterLS.x - bounds.Radius;
		float b = sphereCenterLS.y - bounds.Radius;
		float n = sphereCenterLS.z - bounds.Radius;
		float r = sphereCenterLS.x + bounds.Radius;
		float t = sphereCenterLS.y + bounds.Radius;
		float f = sphereCenterLS.z + bounds.Radius;

		NearZ = n;
		FarZ = f;
		XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2

		XMMATRIX S = view * lightProj * T;
		XMStoreFloat4x4(&View, view);
		XMStoreFloat4x4(&Proj, lightProj);
		XMStoreFloat4x4(&ShadowTransform, S);
	}
	else if (lightType == ELightType::Spot)
	{
		auto globalPos = LightComponent->GetGlobalPosition();
		auto spotLight = LightComponent->GetSpotLight();
		lightPos = globalPos;
		lightDir = XMVector3Normalize(XMLoadFloat3(&spotLight.Direction));
		lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction
		NearZ = spotLight.FalloffStart;
		FarZ = spotLight.FalloffEnd;

		// Create the view matrix for the spotlight
		auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
		auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(spotLight.ConeAngle * 2), 1.0f, NearZ, FarZ);
		XMMATRIX S = view * projection * T;
		XMStoreFloat4x4(&View, view);
		XMStoreFloat4x4(&Proj, projection);
		XMStoreFloat4x4(&ShadowTransform, S);
	}
	XMStoreFloat3(&LightPos, lightPos);
}

void OShadowMap::SetPassConstants()
{
	using namespace DirectX;

	const auto view = XMLoadFloat4x4(&View);
	const auto proj = XMLoadFloat4x4(&Proj);
	const auto shadowTransform = XMLoadFloat4x4(&ShadowTransform);

	XMFLOAT4X4 transposed{};
	Put(transposed, Transpose(shadowTransform));
	Light
	    DirectionalLight.Transform
	    = transposed;
	PointLight.Transform = transposed;
	SpotLight.Transform = transposed;

	const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	const XMMATRIX invView = Inverse(view);
	const XMMATRIX invProj = Inverse(proj);
	const XMMATRIX invViewProj = Inverse(viewProj);

	XMStoreFloat4x4(&PassConstant.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&PassConstant.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&PassConstant.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&PassConstant.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&PassConstant.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&PassConstant.InvViewProj, XMMatrixTranspose(invViewProj));

	PassConstant.EyePosW = LightPos;
	PassConstant.NearZ = NearZ;
	PassConstant.FarZ = FarZ;
}

uint32_t OShadowMap::GetNumDSVRequired() const
{
	return 1;
}

uint32_t OShadowMap::GetNumSRVRequired() const
{
	return 1;
}

uint32_t OShadowMap::GetNumPassesRequired() const
{
	return 1;
}

void OShadowMap::SetLightIndex(uint32_t Idx)
{
	LightComponent->SetShadowMapIndex(Idx);
}

D3D12_GPU_VIRTUAL_ADDRESS OShadowMap::GetPassConstantAddresss() const
{
	return PassConstantBuffer.Buffer->GetGPUAddress() + PassConstantBuffer.StartIndex * Utils::CalcBufferByteSize(sizeof(SPassConstants));
}
