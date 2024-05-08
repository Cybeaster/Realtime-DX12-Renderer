#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/RenderTarget/ShadowMap/ShadowMap.h"

class ODirectionalLightComponent;
/*
 * Cascade shadow map
 */
class OCSM final : public ORenderTargetBase
{
public:
	OCSM(ID3D12Device* Device, DXGI_FORMAT Format);

	void BuildDescriptors() override;
	void BuildResource() override;
	SResourceInfo* GetResource() override;
	SResourceInfo* GetSubresource(uint32_t Idx) override;

	OShadowMap* GetShadowMap(uint32_t Idx);
	const array<OShadowMap*, 3>& GetShadowMaps() const;

private:
	ODirectionalLightComponent* InLightComponent;
	array<OShadowMap*, 3> ShadowMaps;
};
