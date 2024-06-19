#define HLSL
#include "Common.hlsl"


struct GSInput {
	uint InstanceID : SV_InstanceID;
};

struct GSOutput {
	float4 Pos : SV_POSITION;
};

GSInput VS(uint InstanceID
			 : SV_InstanceID)
{
	GSInput output = (GSInput)0;
	output.InstanceID = InstanceID;
	return output;
}

[maxvertexcount(24)]
void GS(point GSInput input[1], inout LineStream<GSOutput> OutputStream) {
	uint instanceID = input[0].InstanceID;
	float3 center = gInstanceData[instanceID].BoundingBoxCenter;
	float3 extents = gInstanceData[instanceID].BoundingBoxExtents;

	// Define the 8 corners of the box
	float3 corners[8] = {
		center + float3(-extents.x, -extents.y, -extents.z),
		center + float3(extents.x, -extents.y, -extents.z),
		center + float3(extents.x, extents.y, -extents.z),
		center + float3(-extents.x, extents.y, -extents.z),
		center + float3(-extents.x, -extents.y, extents.z),
		center + float3(extents.x, -extents.y, extents.z),
		center + float3(extents.x, extents.y, extents.z),
		center + float3(-extents.x, extents.y, extents.z)
	};

	// Lines between the corners to form the box
	uint indices[24] = {
		0, 1, 1, 2, 2, 3, 3, 0,  // Bottom face
		4, 5, 5, 6, 6, 7, 7, 4,  // Top face
		0, 4, 1, 5, 2, 6, 3, 7   // Side edges
	};

	// Output the lines
	for (int i = 0; i < 24; i += 2) {
		GSOutput start, end;
		start.Pos = mul(float4(corners[indices[i]], 1.0f), cbCamera.ViewProj);
		end.Pos = mul(float4(corners[indices[i+1]], 1.0f), cbCamera.ViewProj);
		OutputStream.Append(start);
		OutputStream.Append(end);
		OutputStream.RestartStrip();
	}
}


float4 PS(GSOutput input) : SV_Target
{
	return float4(1.0, 0.0, 0.0, 1.0); // Red color
}