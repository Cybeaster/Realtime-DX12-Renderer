#include "Common.hlsl"


struct VertexOut
{
    float4 dummyPos : SV_POSITION; // This won't actually be used for positioning in this scenario.
};

VertexOut VS()
{
    VertexOut output;
    output.dummyPos = float4(0.0f, 0.0f, 0.0f, 1.0f); // Dummy position
    return output;
}

cbuffer cbCameraMatrix : register(b2)
{
    float4x4 gCamViewProj;
    float4x4 gCamInvViewProj;
};

struct GsOut
{
    float4 posH : SV_POSITION;  // Clip space position for rasterization
    float3 posW : POSITION;     // World space position for debugging or additional effects
};

[maxvertexcount(24)]
void GS(triangle VertexOut input[3], inout LineStream<GsOut> OutputStream)
{
    GsOut output;
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
        worldPos[i] = mul(float4(ndcCorners[i], 1.0f), gInvViewProj); // Transform from light's NDC to world space
    }

    // Indices for drawing lines to form the frustum
    int lineIndices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Connect far plane corners
        4, 5, 5, 6, 6, 7, 7, 4, // Connect near plane corners
        0, 4, 1, 5, 2, 6, 3, 7  // Connect corresponding near and far plane corners
    };

    // Draw lines between the corners
    for (int i = 0; i < 24; i += 2) {
        output.posW = worldPos[lineIndices[i]].xyz;   // World space position for debugging
        output.posH = mul(worldPos[lineIndices[i]], gCamViewProj); // Transform to camera clip space
        OutputStream.Append(output);

        output.posW = worldPos[lineIndices[i + 1]].xyz; // World space position for debugging
        output.posH = mul(worldPos[lineIndices[i + 1]], gCamViewProj); // Transform to camera clip space
        OutputStream.Append(output);
        OutputStream.RestartStrip();
    }
}

float4 PS(GsOut input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0); // Simple red color for visibility
}