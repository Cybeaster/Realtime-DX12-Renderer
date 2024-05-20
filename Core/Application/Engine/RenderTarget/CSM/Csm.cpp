#include "Csm.h"

#include "Engine/Engine.h"

OCSM::OCSM(const weak_ptr<ODevice>& Device, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, SRenderConstants::ShadowMapSize, SRenderConstants::ShadowMapSize, Format, Default)
{
	for (uint32_t i = 0; i < 3; i++)
	{
		ShadowMaps[i] = OEngine::Get()->CreateShadowMap();
	}
}

void OCSM::BuildDescriptors()
{
}

void OCSM::BuildResource()
{
}

SResourceInfo* OCSM::GetResource()
{
	return nullptr;
}

SResourceInfo* OCSM::GetSubresource(uint32_t Idx)
{
	return ShadowMaps[Idx].lock()->GetResource();
}

weak_ptr<OShadowMap> OCSM::GetShadowMap(uint32_t Idx)
{
	return ShadowMaps[Idx];
}

const array<weak_ptr<OShadowMap>, 3>& OCSM::GetShadowMaps() const
{
	return ShadowMaps;
}