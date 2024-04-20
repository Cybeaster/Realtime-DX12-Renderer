#include "FrameResource.h"

SFrameResource::SFrameResource(ID3D12Device* Device, IRenderObject* Owner)
    : Owner(Owner), Device(Device)
{
}

SFrameResource::~SFrameResource()
{
}

void SFrameResource::SetPass(UINT PassCount)
{
	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true, Owner, L"_PassBuffer");
}

void SFrameResource::SetInstances(UINT InstanceCount)
{
	if (InstanceCount == 0)
	{
		LOG(Engine, Warning, "MaxInstanceCount is 0");
		return;
	}
	InstanceBuffer = make_unique<OUploadBuffer<SInstanceData>>(Device, InstanceCount, false, Owner, L"_InstanceBuffer");
}

void SFrameResource::SetMaterials(UINT MaterialCount)
{
	if (MaterialCount > 0)
	{
		MaterialBuffer = make_unique<OUploadBuffer<HLSL::MaterialData>>(Device, MaterialCount, false, Owner, L"_MaterialBuffer");
	}
	else
	{
		LOG(Engine, Warning, "Material count is 0");
	}
}
void SFrameResource::SetDirectionalLight(UINT LightCount)
{
	if (LightCount > 0)
	{
		DirectionalLightBuffer = make_unique<OUploadBuffer<HLSL::DirectionalLight>>(Device, LightCount, false, Owner, L"_DirectionalLightBuffer");
	}
	else
	{
		LOG(Engine, Warning, "Directional light count is 0");
	}
}

void SFrameResource::SetPointLight(UINT LightCount)
{
	if (LightCount > 0)
	{
		PointLightBuffer = make_unique<OUploadBuffer<HLSL::PointLight>>(Device, LightCount, false, Owner, L"_PointLightBuffer");
	}
	else
	{
		LOG(Engine, Warning, "Point light count is 0");
	}
}

void SFrameResource::SetSpotLight(UINT LightCount)
{
	if (LightCount > 0)
	{
		SpotLightBuffer = make_unique<OUploadBuffer<HLSL::SpotLight>>(Device, LightCount, false, Owner, L"_SpotLightBuffer");
	}
	else
	{
		LOG(Engine, Warning, "Point light count is 0");
	}
}
void SFrameResource::SetSSAO()
{
	SsaoCB = make_unique<OUploadBuffer<SSsaoConstants>>(Device, 1, true, Owner, L"_SsaoBuffer");
}