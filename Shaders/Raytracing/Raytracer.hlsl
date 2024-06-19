#include "Common.hlsl"

RaytracingAccelerationStructure TLAS : register(t0, space5);
RWTexture2D<float4> OUTPUT : register(u0, space5);


struct RayPayload
{
    float3 color;
    bool hit;
};

void Run(RayDesc Ray, float2 PixelCoord)
{
	RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
			 RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
			 RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;

	rayQuery.TraceRayInline(TLAS, 0, 0xFF, Ray);
    RayPayload payload = { float3(0.0f, 0.0f, 0.0f), false };

	while(rayQuery.Proceed())
	{
		if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
		{
			float3 hitPosition = rayQuery.WorldRayOrigin() + rayQuery.CommittedRayT() * rayQuery.WorldRayDirection();
            float3 hitNormal = rayQuery.CommittedTriangleFrontFace();
            float2 barycentrics = rayQuery.CommittedTriangleBarycentrics();

            // Simple shading based on normal
            payload.color = 0.5f * (hitNormal + 1.0f); // Normalize normal to range [0,1]
            payload.hit = true;
            break; // Stop after the first hit due to the ACCEPT_FIRST_HIT_AND_END_SEARCH flag

		}
	}

	if (!payload.hit)
	{
		// Miss shader: background color
		payload.color = float3(0.0f, 0.0f, 0.0f); // Black background for misses
	}

	gOutput[PixelCoord] = float4(payload.color, 1.0f);
}


[numthreads(16, 16, 1)]
void RayGeneration(int3 DispatchThreadID
                                     : SV_DispatchThreadID)
{
	uint2 dispatchThreadID = DispatchThreadID.xy;
	uint width, height;
	gOutput.GetDimensions(width, height);

	uint pixelX = dispatchThreadID.x % width;
	uint pixelY = dispatchThreadID.y % height;

	uint hash = pixelX + pixelY * width;
	uint seed = HashUINT(hash);

	float x = (pixelX + GetRandomFloat(seed)) * (1 / width);
	float y = (pixelY + GetRandomFloat(seed)) * (1 / height);

	float angle = tan(0.5f * cbCamera.FOV);
	x = (x * 2.0f - 1.0f) * angle * cbCamera.AspectRatio;
	y = (y * 2.0f - 1.0f) * angle;

	float4 ndc = float4(x, y, 1.0f, 1.0f);
	float4 rayDirView = mul(cbCamera.InvViewProj, ndc);
	rayDirView /= rayDirView.w; // Perspective divide
	float3 rayDirWorld = normalize(rayDirView.xyz);

	float3 pointAimed = cbCamera.EyePosW + rayDirWorld * cbCamera.FocusDistance;
	float2 dofDir = RandomPointInHexagon(seed);
	float apet = cbCamera.Aperture;

	// Calculate the offset in camera space
	float3 right = float3(cbCamera.InvView[0][0], cbCamera.InvView[1][0], cbCamera.InvView[2][0]); // camera right vector
	float3 up = float3(cbCamera.InvView[0][1], cbCamera.InvView[1][1], cbCamera.InvView[2][1]); // camera up vector
	float3 newPos = cbCamera.EyePosW + dofDir.x * apet * right + dofDir.y * apet * up;

	RayDesc ray;
	ray.Origin = newPos;
	ray.Direction = normalize(pointAimed - newPos);
	ray.TMin = 0.001f; // Start a bit away from the origin to avoid self-intersections
	ray.TMax = 10000.0f; // Set a maximum distance for the ray

	Run(ray, float2(pixelX, pixelY));
}