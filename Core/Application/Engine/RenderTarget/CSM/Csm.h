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
	OCSM(const weak_ptr<ODevice>& Device, DXGI_FORMAT Format);

	void BuildDescriptors() override;
	void BuildResource() override;
	SResourceInfo* GetResource() override;
	SResourceInfo* GetSubresource(uint32_t Idx) override;

	weak_ptr<OShadowMap> GetShadowMap(uint32_t Idx);
	const array<weak_ptr<OShadowMap>, 3>& GetShadowMaps() const;

private:
	ODirectionalLightComponent* InLightComponent;
	array<weak_ptr<OShadowMap>, 3> ShadowMaps;
};
