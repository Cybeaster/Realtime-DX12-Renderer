#include "RaytracingCommon.hlsl"

[shader("miss")]
void Miss(inout HitInfo payload : SV_RayPayload)
{
    payload.Color = float4(0.2f, 0.2f, 0.8f, -1.f);
}