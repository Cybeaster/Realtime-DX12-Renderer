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
	PassCB = make_unique<OUploadBuffer<SPassConstants>>(Device, PassCount, true, Owner);
}
 void SFrameResource::SetInstances(UINT InstanceCount)
{
	if (InstanceCount == 0)
	{
		LOG(Engine, Warning, "MaxInstanceCount is 0");
		return;
	}
	InstanceBuffer = make_unique<OUploadBuffer<SInstanceData>>(Device, InstanceCount, false, Owner);
}

 void SFrameResource::SetMaterials(UINT MaterialCount)
{
	if (MaterialCount > 0)
	{
		MaterialBuffer = make_unique<OUploadBuffer<SMaterialData>>(Device, MaterialCount, false, Owner);
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
		DirectionalLightBuffer = make_unique<OUploadBuffer<SDirectionalLight>>(Device, LightCount, false, Owner);
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
 		PointLightBuffer = make_unique<OUploadBuffer<SPointLight>>(Device, LightCount, false, Owner);
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
 		SpotLightBuffer = make_unique<OUploadBuffer<SSpotLight>>(Device, LightCount, false, Owner);
 	}
 	else
 	{
 		LOG(Engine, Warning, "Point light count is 0");
 	}
}