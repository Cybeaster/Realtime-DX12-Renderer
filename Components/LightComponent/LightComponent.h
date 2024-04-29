
#pragma once
#include "DirectX/HlslTypes.h"
#include "DirectX/Light/Light.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "RenderItemComponentBase.h"

class OCSM;
class OShadowMap;
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
	OLightComponent();

	void Tick(UpdateEventArgs Arg) override;
	void Init(ORenderItem* Other) override;
	virtual void SetLightSourceData() = 0;

	int NumFramesDirty = SRenderConstants::NumFrameResources;

	virtual void SetPassConstant(SPassConstants& OutConstant) = 0;
	virtual ELightType GetLightType() const = 0;
	virtual void UpdateFrameResource(const SFrameResource* FrameResource) = 0;

	void UpdateLightData();

	virtual int32_t GetLightIndex() const = 0;
	DirectX::XMVECTOR GetGlobalPosition() const;
	void MarkDirty();
	bool UseDirty();
	bool TryUpdate();

	SDelegate<void> OnLightChanged;

private:
	bool bIsDirty = true;

	DirectX::XMVECTOR GlobalPosition;
};

//Directional Light component
class ODirectionalLightComponent final : public OLightComponent
{
public:
	ODirectionalLightComponent(uint32_t InIdx);
	void SetLightSourceData() override;
	void SetShadowMapIndices(array<UINT, MAX_CSM_PER_FRAME> Maps);
	HLSL::DirectionalLight& GetDirectionalLight();
	ELightType GetLightType() const override;
	void SetDirectionalLight(const SDirectionalLightPayload& Light);
	int32_t GetLightIndex() const override;
	void UpdateFrameResource(const SFrameResource* FrameResource) override;
	void InitFrameResource(const TUploadBufferData<HLSL::DirectionalLight>& Spot);
	void SetPassConstant(SPassConstants& OutConstant) override;
	void SetCSM(OCSM* InCSM);

private:
	array<HLSL::ShadowMapData*, 3> MakeShadowMapDataArray();
	TUploadBufferData<HLSL::DirectionalLight> DirLightBufferInfo;
	HLSL::DirectionalLight DirectionalLight;
	OCSM* CSM = nullptr;
};

//Point light component
class OPointLightComponent final : public OLightComponent
{
public:
	OPointLightComponent(uint32_t InIdx);
	ELightType GetLightType() const override;
	HLSL::PointLight& GetPointLight();
	void SetPointLight(const SPointLightPayload& Light);
	void SetShadowMapIndex(UINT Index);
	int32_t GetLightIndex() const override;
	void UpdateFrameResource(const SFrameResource* FrameResource) override;
	void InitFrameResource(const TUploadBufferData<HLSL::PointLight>& Spot);
	void SetPassConstant(SPassConstants& OutConstant) override;
	void SetLightSourceData() override;

private:
	HLSL::PointLight PointLight = {};
	TUploadBufferData<HLSL::PointLight> PointLightBufferInfo;
	OShadowMap* ShadowMap = {};
};

class OSpotLightComponent final : public OLightComponent
{
public:
	OSpotLightComponent(uint32_t InIdx);
	void SetSpotLight(const SSpotLightPayload& Light);
	void SetShadowMapIndex(UINT Index);
	int32_t GetLightIndex() const override;
	void UpdateFrameResource(const SFrameResource* FrameResource) override;
	void InitFrameResource(const TUploadBufferData<HLSL::SpotLight>& Spot);
	ELightType GetLightType() const override;
	void SetPassConstant(SPassConstants& OutConstant) override;
	void SetLightSourceData() override;
	HLSL::SpotLight& GetSpotLight();
	void SetShadowMap(OShadowMap* InShadow);

private:
	TUploadBufferData<HLSL::SpotLight> SpotLightBufferInfo;
	HLSL::SpotLight SpotLight = {};
	OShadowMap* ShadowMap = {};
};