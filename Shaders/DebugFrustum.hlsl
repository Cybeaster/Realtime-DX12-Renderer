#include "Common.hlsl"



struct GSInput {
	uint InstanceID : SV_InstanceID;
};

GSInput VS(uint InstanceID : SV_InstanceID)
{
	GSInput output;
	output.InstanceID = InstanceID;
	return output;
}

struct GSOutput {
	float4 PosH : SV_POSITION;
	nointerpolation float3 Color : COLOR;
};

[maxvertexcount(24)]
void GS(point GSInput input[1], inout LineStream<GSOutput> OutputStream)
{
    GSOutput output;

    uint instanceID = input[0].InstanceID;
    float4x4 invViewProj = gInstanceData[instanceID].InvViewProjection;
	output.Color = gInstanceData[instanceID].OverrideColor; // Red color
    // Define frustum corners in NDC
    float3 ndcCorners[8] = {
        {-1.0f, 1.0f, -1.0f},  // Top-left-far
        {1.0f, 1.0f, -1.0f},   // Top-right-far
        {1.0f, -1.0f, -1.0f},  // Bottom-right-far
        {-1.0f, -1.0f, -1.0f}, // Bottom-left-far
        {-1.0f, 1.0f, 1.0f},   // Top-left-near
        {1.0f, 1.0f, 1.0f},    // Top-right-near
        {1.0f, -1.0f, 1.0f},   // Bottom-right-near
        {-1.0f, -1.0f, 1.0f}   // Bottom-left-near
    };

    // Transform NDC corners to world space using the inverse view-projection matrix of the light
    float4 worldPos[8];
    for (int i = 0; i < 8; ++i) {
        worldPos[i] = mul(float4(ndcCorners[i], 1.0f), invViewProj); // Transform from light's NDC to world space
    }

    // Indices for drawing lines to form the frustum
    int lineIndices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Connect far plane corners
        4, 5, 5, 6, 6, 7, 7, 4, // Connect near plane corners
        0, 4, 1, 5, 2, 6, 3, 7  // Connect corresponding near and far plane corners
    };

    // Draw lines between the corners
    for (int i = 0; i < 24; i += 2) {
        output.PosH = mul(worldPos[lineIndices[i]], cbCamera.ViewProj); // Transform to camera clip space
        OutputStream.Append(output);

        output.PosH = mul(worldPos[lineIndices[i + 1]], cbCamera.ViewProj); // Transform to camera clip space
        OutputStream.Append(output);
        OutputStream.RestartStrip();
    }
}

float4 PS(GSOutput input) : SV_TARGET
{
    return float4(input.Color,1);
}