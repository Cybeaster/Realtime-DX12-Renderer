#include "Common.hlsl"

RaytracingAccelerationStructure TLAS : register(t0, space5);
RWTexture2D<float4> OUTPUT : register(u0, space5);

void Run(RayDesc Ray, float2 PixelCoord)
{
	RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
			 RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
			 RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;

	rayQuery.TraceRayInline(TLAS,
	 RAY_FLAG_CULL_NON_OPAQUE,
	 0xFF,
	 0,
	 Ray);

	while(rayQuery.Proceed())
	{
		if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
		{
			float3 hitPosition = rayQuery.CommittedTriangleWorldPosition();
			float3 hitNormal = rayQuery.CommittedTriangleWorldNormal();
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


[numthreads(16,16,1)] void RayGen()
{
	uint2 dispatchThreadID = DispatchRaysIndex().xy;
	uint width, height;
	gOutput.GetDimensions(width, height);

	uint pixelX = dispatchThreadID.x % width;
	uint pixelY = dispatchThreadID.y % height;

	uint seed = HashUINT(pixelX + pixelY * width);

	float x = (pixelX + GetRandomFloat(seed)) * (1 / width);
	float y = (pixelY + GetRandomFloat(seed)) * (1 / height);

	float angle = tan(0.5f * gCamera.FOV);
	x = (x * 2.0f - 1.0f) * angle * gCamera.AspectRatio;
	y = (y * 2.0f - 1.0f) * angle;

	float4 ndc = float4(x, y, 1.0f, 1.0f);
	float4 rayDirView = mul(gCamera.gInvViewProj, ndc);
	rayDirView /= rayDirView.w; // Perspective divide
	float3 rayDirWorld = normalize(rayDirView.xyz);

	float3 pointAimed = gCamera.Position + rayDirWorld * gCamera.FocusDistance;
	float2 dofDir = RandomPointInHexagon(seed);
	float apet = gCamera.Aperture;

	// Calculate the offset in camera space
	float3 right = float3(gCamera.gInvView[0][0], gCamera.gInvView[1][0], gCamera.gInvView[2][0]); // camera right vector
	float3 up = float3(gCamera.gInvView[0][1], gCamera.gInvView[1][1], gCamera.gInvView[2][1]); // camera up vector
	float3 newPos = gCamera.gEyePosW + dofDir.x * apet * right + dofDir.y * apet * up;

	RayDesc ray;
	ray.Origin = newPos;
	ray.Direction = normalize(pointAimed - newPos);
	ray.TMin = 0.001f; // Start a bit away from the origin to avoid self-intersections
	ray.TMax = 10000.0f; // Set a maximum distance for the ray

	RunRT(ray);
}