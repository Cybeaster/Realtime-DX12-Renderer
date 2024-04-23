
#pragma once
#include "DirectX/HlslTypes.h"
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

	void SetDirectionalLight(const SDirectionalLightPayload& Light);
	void SetPointLight(const SPointLightPayload& Light);
	void SetSpotLight(const SSpotLightPayload& Light);

	void Init(ORenderItem* Other) override;
	void UpdateFrameResource(const SFrameResource* FrameResource);
	void InitFrameResource(const TUploadBufferData<HLSL::DirectionalLight>& Dir,
	                       const TUploadBufferData<HLSL::PointLight>& Point,
	                       const TUploadBufferData<HLSL::SpotLight>& Spot);
	DirectX::XMVECTOR GetGlobalPosition() const;
	int NumFramesDirty = SRenderConstants::NumFrameResources;
	bool TryUpdate();
	ELightType GetLightType() const;
	int32_t GetLightIndex() const;
	HLSL::DirectionalLight& GetDirectionalLight();
	HLSL::PointLight& GetPointLight();
	HLSL::SpotLight& GetSpotLight();
	void MarkDirty();
	void SetPassConstant(SPassConstants& OutConstant);
	void UpdateLightData();
	void SetShadowMapIndex(UINT Index);
	SDelegate<void> OnLightChanged;

	bool UseDirty()
	{
		if (bIsDirty)
		{
			bIsDirty = false;
			return true;
		}
		return false;
	}

private:
	bool bIsDirty = true;
	ELightType LightType = ELightType::Spot;
	TUploadBufferData<HLSL::DirectionalLight> DirLightBufferInfo;
	TUploadBufferData<HLSL::PointLight> PointLightBufferInfo;
	TUploadBufferData<HLSL::SpotLight> SpotLightBufferInfo;
	DirectX::XMVECTOR GlobalPosition;

	HLSL::DirectionalLight DirectionalLight;
	HLSL::PointLight PointLight;
	HLSL::SpotLight SpotLight;

};
