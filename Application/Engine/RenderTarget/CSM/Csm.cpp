#include "Csm.h"

#include "Engine/Engine.h"

OCSM::OCSM(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format)
    : ORenderTargetBase(Device, Width, Height, Format, Default)
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
	return ShadowMaps[Idx]->GetResource();
}

OShadowMap* OCSM::GetShadowMap(uint32_t Idx)
{
	return ShadowMaps[Idx];
}

const array<OShadowMap*, 3>& OCSM::GetShadowMaps() const
{
	return ShadowMaps;
}