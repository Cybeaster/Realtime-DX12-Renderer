#include "Common.hlsl"

RaytracingAccelerationStructure TLAS : register(t5, space0);
RWTexture2D<float4> OUTPUT : register(u0, space5);
StructuredBuffer<VertexData> VERTEX_DATA : register(t5, space1);


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

	rayQuery.TraceRayInline(TLAS, 0, 0, Ray);
    RayPayload payload = { float3(0.0f, 0.0f, 0.0f), false };

//	while(rayQuery.Proceed())
	{
		if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
		{
			// Get the hit position and normal
			InstanceData instance = INSTANCE_DATA[rayQuery.CommittedInstanceIndex()];
			uint primitiveIndex = rayQuery.CommittedPrimitiveIndex();
			uint baseVertIndex = primitiveIndex + instance.StartVertexLocation;

			float3 vertex_1 = VERTEX_DATA[baseVertIndex].Position;
			float3 vertex_2 = VERTEX_DATA[baseVertIndex + 1].Position;
			float3 vertex_3 = VERTEX_DATA[baseVertIndex + 2].Position;

			float3 edge1 = vertex_2 - vertex_1;
			float3 edge2 = vertex_3 - vertex_1;
			float3 hitNormal = normalize(cross(edge1, edge2));
			float3 hitPosition = rayQuery.WorldRayOrigin() + rayQuery.CommittedRayT() * rayQuery.WorldRayDirection();
            float2 barycentrics = rayQuery.CommittedTriangleBarycentrics();

            // Simple shading based on normal
            payload.color = float3(0.5f, 0.5f, 0.5f) + 0.5f * hitNormal;
            payload.hit = true;
      //      break; // Stop after the first hit due to the ACCEPT_FIRST_HIT_AND_END_SEARCH flag

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
void RayGeneration(int3 DispatchThreadID : SV_DispatchThreadID)
{
	uint2 dispatchThreadID = DispatchThreadID.xy;
	uint width, height;
	gOutput.GetDimensions(width, height);

	uint pixelX = dispatchThreadID.x % width;
	uint pixelY = dispatchThreadID.y % height;

	uint hash = pixelX + pixelY * width;
	uint seed = HashUINT(hash); // Ensure the seed is defined here

	// Normalize pixel coordinates with added randomness
	float x = (pixelX) / width;
	float y = (pixelY ) / height;

	// Convert to NDC space [-1, 1]
	x = (x * 2.0f - 1.0f) * tan(0.5f * cbCamera.FOV) * cbCamera.AspectRatio;
	y = (y * 2.0f - 1.0f) * tan(0.5f * cbCamera.FOV);

	float4 ndc = float4(x, y, 1.0f, 1.0f);
	float4 rayDirView = mul(cbCamera.InvViewProj, ndc);
	rayDirView /= rayDirView.w; // Perspective divide
	float3 rayDirWorld = normalize(rayDirView.xyz);

	float3 pointAimed = cbCamera.EyePosW + rayDirWorld;
	float2 dofDir = RandomPointInHexagon(seed);
	float apet = cbCamera.Aperture;

	// Calculate the offset in camera space for depth of field (DOF)
	float3 right = float3(cbCamera.InvView[0][0], cbCamera.InvView[1][0], cbCamera.InvView[2][0]); // camera right vector
	float3 up = float3(cbCamera.InvView[0][1], cbCamera.InvView[1][1], cbCamera.InvView[2][1]); // camera up vector
	float3 newPos = cbCamera.EyePosW + dofDir.x * apet * right + dofDir.y * apet * up;

	// Define the ray
	RayDesc ray;
	ray.Origin = newPos;
	ray.Direction = normalize(pointAimed - newPos);
	ray.TMin = 0.001f; // Start a bit away from the origin to avoid self-intersections
	ray.TMax = 10000.0f; // Set a maximum distance for the ray

	// Run the ray tracing function with the ray and pixel coordinates
	Run(ray, float2(dispatchThreadID.x, dispatchThreadID.y));
}

