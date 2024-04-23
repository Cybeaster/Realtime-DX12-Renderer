#include "Csm.h"

OCSM::OCSM(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, EResourceHeapType HeapType)
    : ORenderTargetBase(Device, Width, Height, Format, HeapType)
{

}

void OCSM::BuildDescriptors()
{
}

void OCSM::BuildResource()
{
}

SResourceInfo* OCSM::GetResource()
{
}