
#pragma once
#include "DirectX/Light/Light.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "RenderItemComponentBase.h"

struct SFrameResource;
ENUM(ELightType,
     Directional,
     Point,
     Spot)

inline string ToString(const ELightType& Type)
{
	switch (Type)
	{
	case ELightType::Directional:
		return "Directional";
		break;
	case ELightType::Point:
		return "Point";
		break;
	case ELightType::Spot:
		return "Spot";
		break;
	}
	return "Unknown";
}

class OLightComponent : public OSceneComponent
{
public:
	OLightComponent() = default;
	OLightComponent(uint32_t DirLightIdx, uint32_t PointLightIdx, uint32_t SpotLightIdx, ELightType Type);

	void Tick(UpdateEventArgs Arg) override;
	void SetDirectionalLight(const SDirectionalLight& Light);
	void SetPointLight(const SPointLight& Light);
	void SetSpotLight(const SSpotLight& Light);
	void Init(ORenderItem* Other) override;
	void UpdateFrameResource(const SFrameResource* FrameResource);
	void InitFrameResource(const TUploadBufferData<SDirectionalLight>& Dir,
	                       const TUploadBufferData<SPointLight>& Point,
	                       const TUploadBufferData<SSpotLight>& Spot);

	int NumFramesDirty = SRenderConstants::NumFrameResources;
	bool TryUpdate();
	ELightType GetLightType() const;
	int32_t GetLightIndex() const;
	SDirectionalLight& GetDirectionalLight();
	SPointLight& GetPointLight();
	SSpotLight& GetSpotLight();
	void MarkDirty();
	void SetPassConstant(SPassConstants& OutConstant);
	void UpdateLightData();
	void SetShadowMapIndex(UINT Index);

private:
	ELightType LightType = ELightType::Spot;

	TUploadBufferData<SDirectionalLight> DirLightBufferInfo;
	TUploadBufferData<SPointLight> PointLightBufferInfo;
	TUploadBufferData<SSpotLight> SpotLightBufferInfo;
	DirectX::XMVECTOR GlobalPosition;

	SDirectionalLight DirectionalLight;
	SPointLight PointLight;
	SSpotLight SpotLight;

	DirectX::XMFLOAT3 LightPos;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Proj;
	DirectX::XMFLOAT4X4 ShadowTransform;
	float NearZ;
	float FarZ;
};
