#include "RaytracingCommon.hlsl"

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib) 
{
  payload.Color = float4(1, 1, 0, RayTCurrent());
}
