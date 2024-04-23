
#include "LightComponent.h"

#include "Engine/Engine.h"
#include "Profiler.h"

OLightComponent::OLightComponent(uint32_t DirLightIdx, uint32_t PointLightIdx, uint32_t SpotLightIdx, ELightType Type)
    : DirLightBufferInfo(DirLightIdx), PointLightBufferInfo(PointLightIdx), SpotLightBufferInfo(SpotLightIdx), LightType(Type)
{
	Name = "LightComponent";
}

void OLightComponent::Tick(UpdateEventArgs Arg)
{
	PROFILE_SCOPE();
	if (!Utils::MatricesEqual(Owner->GetDefaultInstance()->World, Position)) // make delegate based
	{
		DirectX::XMVECTOR scale, rotation, translation;
		DirectX::XMMatrixDecompose(&scale, &rotation, &translation, DirectX::XMLoadFloat4x4(&Owner->GetDefaultInstance()->World));
		Position = Owner->GetDefaultInstance()->World;
		DirectX::XMFLOAT4 pos{};
		DirectX::XMStoreFloat4(&pos, translation);
		GlobalPosition = translation;

		SpotLight.Position = { pos.x, pos.y, pos.z };
		PointLight.Position = { pos.x, pos.y, pos.z };
		OnLightChanged.Broadcast();
		MarkDirty();
	}
	UpdateLightData();
}

void OLightComponent::SetDirectionalLight(const SDirectionalLightPayload& Light)
{
	LightType = ELightType::Directional;
	DirectionalLight.Direction = Light.Direction;
	DirectionalLight.Intensity = Light.Intensity;
	UpdateLightData();
}

void OLightComponent::SetPointLight(const SPointLightPayload& Light)
{
	LightType = ELightType::Point;
	PointLight.FalloffStart = Light.FallOffStart;
	PointLight.FalloffEnd = Light.FallOffEnd;
	PointLight.Position = Light.Position;
	PointLight.Intensity = Light.Intensity;
	UpdateLightData();
}

void OLightComponent::SetSpotLight(const SSpotLightPayload& Light)
{
	LightType = ELightType::Spot;
	SpotLight.ConeAngle = Light.ConeAngle;
	SpotLight.Direction = Light.Direction;
	SpotLight.FalloffEnd = Light.FallOffEnd;
	SpotLight.FalloffStart = Light.FallOffStart;
	SpotLight.SpotPower = Light.SpotPower;
	SpotLight.Intensity = Light.Strength;
	UpdateLightData();
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

void OLightComponent::InitFrameResource(const TUploadBufferData<HLSL::DirectionalLight>& Dir, const TUploadBufferData<HLSL::PointLight>& Point, const TUploadBufferData<HLSL::SpotLight>& Spot)
{
	DirLightBufferInfo = Dir;
	PointLightBufferInfo = Point;
	SpotLightBufferInfo = Spot;
}

DirectX::XMVECTOR OLightComponent::GetGlobalPosition() const
{
	return GlobalPosition;
}

bool OLightComponent::TryUpdate()
{
	bool res = NumFramesDirty > 0;
	NumFramesDirty = 0;
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

HLSL::DirectionalLight& OLightComponent::GetDirectionalLight()
{
	return DirectionalLight;
}

HLSL::PointLight& OLightComponent::GetPointLight()
{
	return PointLight;
}

HLSL::SpotLight& OLightComponent::GetSpotLight()
{
	return SpotLight;
}

void OLightComponent::MarkDirty()
{
	LOG(Render, Log, "Marking light as dirty");
	NumFramesDirty = SRenderConstants::NumFrameResources;
	OnLightChanged.Broadcast();
}

void OLightComponent::UpdateLightData()
{
	PROFILE_SCOPE();
	bIsDirty = true;
}

void OLightComponent::SetShadowMapIndex(UINT Index)
{
	DirectionalLight.ShadowMapIndex = Index;
	PointLight.ShadowMapIndex = Index;
	SpotLight.ShadowMapIndex = Index;
}
