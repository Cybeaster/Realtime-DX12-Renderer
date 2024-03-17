
#include "LightComponent.h"

#include "Engine/Engine.h"

OLightComponent::OLightComponent(uint32_t DirLightIdx, uint32_t PointLightIdx, uint32_t SpotLightIdx, ELightType Type)
    : DirLightBufferInfo(DirLightIdx), PointLightBufferInfo(PointLightIdx), SpotLightBufferInfo(SpotLightIdx), LightType(Type)
{
	Name = "LightComponent";
}

void OLightComponent::Tick(UpdateEventArgs Arg)
{

	if(!Utils::MatricesEqual(Owner->GetDefaultInstance()->World , Position))
	{
		Position = Owner->GetDefaultInstance()->World;
		NumFramesDirty = SRenderConstants::NumFrameResources;
	}
	else
	{
		NumFramesDirty = 0;
	}
}

void OLightComponent::SetDirectionalLight(const SDirectionalLight& Light)
{
	LightType = ELightType::Directional;
	DirectionalLight = Light;
}

void OLightComponent::SetPointLight(const SPointLight& Light)
{
	LightType = ELightType::Point;
	PointLight = Light;
}

void OLightComponent::SetSpotLight(const SSpotLight& Light)
{
	LightType = ELightType::Spot;
	SpotLight = Light;
}

void OLightComponent::Init(ORenderItem* Other)
{
	OSceneComponent::Init(Other);
}

void OLightComponent::UpdateFrameResource(const SFrameResource* FrameResource)
{
	switch (LightType)
	{
	case ELightType::Directional:
		FrameResource->DirectionalLightBuffer->CopyData(DirLightBufferInfo.StartIndex, DirectionalLight);
		break;
	case ELightType::Point:
		FrameResource->PointLightBuffer->CopyData(PointLightBufferInfo.StartIndex, PointLight);
		break;
	case ELightType::Spot:
		FrameResource->SpotLightBuffer->CopyData(SpotLightBufferInfo.StartIndex, SpotLight);
		break;
	}
}

void OLightComponent::InitFrameResource(const TUploadBufferData<SDirectionalLight>& Dir, const TUploadBufferData<SPointLight>& Point, const TUploadBufferData<SSpotLight>& Spot)
{
	DirLightBufferInfo = Dir;
	PointLightBufferInfo = Point;
	SpotLightBufferInfo = Spot;
}

bool OLightComponent::TryUpdate()
{
	bool res = NumFramesDirty > 0;
	NumFramesDirty=0;
	return res;
}

ELightType OLightComponent::GetLightType() const
{
	return LightType;
}

int32_t OLightComponent::GetLightIndex() const
{
	switch (LightType)
	{
	case ELightType::Directional:
		return DirLightBufferInfo.StartIndex;
		break;
	case ELightType::Point:
		return PointLightBufferInfo.StartIndex;
		break;
	case ELightType::Spot:
		return SpotLightBufferInfo.StartIndex;
		break;
	}
	LOG(Render, Error, "Light type not initized")
	return -1;
}

SDirectionalLight& OLightComponent::GetDirectionalLight()
{
	return DirectionalLight;
}

SPointLight& OLightComponent::GetPointLight()
{
	return PointLight;
}

SSpotLight& OLightComponent::GetSpotLight()
{
	return SpotLight;
}

void OLightComponent::MarkDirty()
{
	NumFramesDirty = SRenderConstants::NumFrameResources;
}
