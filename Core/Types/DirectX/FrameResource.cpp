#include "FrameResource.h"

#include "Statics.h"

SFrameResource::SFrameResource(weak_ptr<ODevice> Device, weak_ptr<IRenderObject> Owner)
    : Owner(Owner), Device(Device)
{
	CameraMatrixBuffer = OUploadBuffer<HLSL::CameraMatrixBuffer>::Create(Device, 1, true, Owner, L"_CameraMatrixBuffer");
}

SFrameResource::~SFrameResource()
{
}

void SFrameResource::SetPass(UINT PassCount)
{
	if (CameraBuffer)
	{
		CameraBuffer->RebuildBuffer(PassCount);
	}
	else
	{
		CameraBuffer = make_unique<OUploadBuffer<HLSL::CameraCBuffer>>(Device, PassCount, true, Owner, L"_PassBuffer");
	}
}

void SFrameResource::SetMaterials(UINT MaterialCount)
{
	if (MaterialCount > 0)
	{
		if (MaterialBuffer)
		{
			MaterialBuffer->RebuildBuffer(MaterialCount);
		}
		else
		{
			MaterialBuffer = make_unique<OUploadBuffer<HLSL::MaterialData>>(Device, MaterialCount, false, Owner, L"_MaterialBuffer");
		}
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
		if (DirectionalLightBuffer)
		{
			DirectionalLightBuffer->RebuildBuffer(LightCount);
		}
		else
		{
			DirectionalLightBuffer = make_unique<OUploadBuffer<HLSL::DirectionalLight>>(Device, LightCount, false, Owner, L"_DirectionalLightBuffer");
		}
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
		if (PointLightBuffer)
		{
			PointLightBuffer->RebuildBuffer(LightCount);
		}
		else
		{
			PointLightBuffer = make_unique<OUploadBuffer<HLSL::PointLight>>(Device, LightCount, false, Owner, L"_PointLightBuffer");
		}
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
		if (SpotLightBuffer)
		{
			SpotLightBuffer->RebuildBuffer(LightCount);
		}
		else
		{
			SpotLightBuffer = make_unique<OUploadBuffer<HLSL::SpotLight>>(Device, LightCount, false, Owner, L"_SpotLightBuffer");
		}
	}
	else
	{
		LOG(Engine, Warning, "Point light count is 0");
	}
}
void SFrameResource::SetSSAO()
{
	if (SsaoCB == nullptr)
	{
		SsaoCB = make_unique<OUploadBuffer<SSsaoConstants>>(Device, 1, true, Owner, L"_SsaoBuffer");
	}
}

void SFrameResource::SetFrusturmCorners()
{
	if (FrusturmCornersBuffer == nullptr)
	{
		FrusturmCornersBuffer = make_unique<OUploadBuffer<HLSL::FrustrumCorners>>(Device, 1, true, Owner, L"_FrusturmCornersBuffer");
	}
}

void SFrameResource::RebuildInstanceBuffers(UINT InstanceCount) const
{
	for (const auto& val : InstanceBuffers | std::views::values)
	{
		val->RebuildBuffer(InstanceCount);
	}
}

OUploadBuffer<HLSL::InstanceData>* SFrameResource::AddNewInstanceBuffer(const wstring& Name, UINT InstanceCount, TUUID Id)
{
	if (InstanceCount == 0)
	{
		LOG(Engine, Warning, "MaxInstanceCount is 0");
		return nullptr;
	}

	InstanceBuffers[Id] = make_unique<OUploadBuffer<HLSL::InstanceData>>(Device, InstanceCount, false, Owner, Name);
	LOG(Engine, Log, "Adding new instance buffer: {}, New Size: {}", Name, TEXT(InstanceBuffers.size()));

	return InstanceBuffers[Id].get();
}