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

struct GsOut
{
    float4 posH : SV_POSITION;  // Clip space position for rasterization
    float3 posW : POSITION;     // World space position for debugging or additional effects
};

[maxvertexcount(24)]
void GS(point VertexOut input[1], inout LineStream<GsOut> OutputStream)
{
    GsOut output;
    // Define frustum corners in NDC
    float3 ndcCorners[8] = {
        float3(-1.0f, 1.0f, -1.0f),
        float3(1.0f, 1.0f, -1.0f),
        float3(1.0f, -1.0f, -1.0f),
        float3(-1.0f, -1.0f, -1.0f),
        float3(-1.0f, 1.0f, 1.0f),
        float3(1.0f, 1.0f, 1.0f),
        float3(1.0f, -1.0f, 1.0f),
        float3(-1.0f, -1.0f, 1.0f)
    };

    // Indices for drawing lines
    int lineIndices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Near plane
        4, 5, 5, 6, 6, 7, 7, 4, // Far plane
        0, 4, 1, 5, 2, 6, 3, 7  // Connecting lines between near and far planes
    };

    // Transform NDC corners to world space using the inverse view-projection matrix
    float4x4 invVP = gInvViewProj;
    float4 worldPos[8];
    for (int i = 0; i < 8; ++i) {
        worldPos[i] = mul(float4(ndcCorners[i], 1.0f), invVP);
    }

    // Draw lines between the corners
    for (int i = 0; i < 24; i += 2) {
        output.posW = worldPos[lineIndices[i]].xyz;
        output.posH = mul(worldPos[lineIndices[i]], gViewProj);
        OutputStream.Append(output);

        output.posW = worldPos[lineIndices[i + 1]].xyz;
        output.posH = mul(worldPos[lineIndices[i + 1]], gViewProj);
        OutputStream.Append(output);
        OutputStream.RestartStrip();
    }
}


float4 PS(GsOut input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0); // Simple red color for visibility
}