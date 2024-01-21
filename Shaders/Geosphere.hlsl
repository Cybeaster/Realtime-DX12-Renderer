
// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.hlsl"
#include "GeometryUtils.hlsl"
Texture2D    gDiffuseMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
	float4x4 gTexTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

	// Allow application to change fog parameters once per frame.
	// For example, we may only use fog for certain times of day.
	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerPassPad2;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
	float4   gDiffuseAlbedo;
    float3   gFresnelR0;
    float    gRoughness;
	float4x4 gMatTransform;
};

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct GeometryOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
	uint PrimID    : SV_PrimitiveID;
};


VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, gMatTransform).xy;

    return vout;
}

void EmitTriangle(VertexOut v0, VertexOut v1, VertexOut v2, inout TriangleStream<GeometryOut> stream) {
	GeometryOut gout = (GeometryOut)0.0f;

	// First vertex
	gout.PosH = v0.PosH;
	gout.PosW = v0.PosW;
	gout.NormalW = v0.NormalW;
	gout.TexC = v0.TexC;
	stream.Append(gout);

	// Second vertex
	gout.PosH = v1.PosH;
	gout.PosW = v1.PosW;
	gout.NormalW = v1.NormalW;
	gout.TexC = v1.TexC;
	stream.Append(gout);

	// Third vertex
	gout.PosH = v2.PosH;
	gout.PosW = v2.PosW;
	gout.NormalW = v2.NormalW;
	gout.TexC = v2.TexC;

	stream.Append(gout);
}


VertexOut Midpoint(VertexOut v0, VertexOut v1)
{
	VertexOut mid= (VertexOut)0.0f;

	// Interpolate positions in world space
	mid.PosW = (v0.PosW + v1.PosW) * 0.5;

	// Interpolate normals and normalize them
	mid.NormalW = normalize((v0.NormalW + v1.NormalW) * 0.5);

	// Interpolate texture coordinates
	mid.TexC = (v0.TexC + v1.TexC) * 0.5;

	mid.PosH = mul(float4(mid.PosW, 1.0f), gViewProj);

	return mid;
}


#define MAX_VERTICES 32 // Adjust this based on your maximum expected triangles

void SubdivideIcosahedron(VertexOut v0, VertexOut v1, VertexOut v2, inout TriangleStream<GeometryOut> stream, int lod) {
    // Static array to store intermediate vertices
    VertexOut vertices[MAX_VERTICES];
    int numVertices = 0;

    // Add initial vertices
    vertices[numVertices++] = v0;
    vertices[numVertices++] = v1;
    vertices[numVertices++] = v2;

    for (int level = 0; level < lod; ++level) {
        int currentNumVertices = numVertices;
        numVertices = 0;
      // Inside the for loop for subdivision
		for (int i = 0; i < currentNumVertices; i += 3) {
		    // Extract vertices of the current triangle
		    VertexOut v0 = vertices[i];
		    VertexOut v1 = vertices[i + 1];
		    VertexOut v2 = vertices[i + 2];

		    // Calculate midpoints
		    VertexOut v01 = Midpoint(v0, v1);
		    VertexOut v12 = Midpoint(v1, v2);
		    VertexOut v20 = Midpoint(v2, v0);

		    // Add new triangles to the array with correct winding order
		    vertices[numVertices++] = v0;
		    vertices[numVertices++] = v01;
		    vertices[numVertices++] = v20;

		    vertices[numVertices++] = v01;
		    vertices[numVertices++] = v1;
		    vertices[numVertices++] = v12;

		    vertices[numVertices++] = v20;
		    vertices[numVertices++] = v01;
		    vertices[numVertices++] = v12;

		    vertices[numVertices++] = v20;
		    vertices[numVertices++] = v12;
		    vertices[numVertices++] = v2;
		}
    }

    // Emit final triangles
    for (int i = 0; i < numVertices; i += 3) {
        EmitTriangle(vertices[i], vertices[i + 1], vertices[i + 2], stream);
    }
}





[maxvertexcount(32)] // Adjust based on maximum expected vertices after subdivision
void GS(triangle VertexOut gin[3], uint primID : SV_PrimitiveID, inout TriangleStream<GeometryOut> stream)
{
	// Calculate the distance from the camera to the center of the triangle
	float distance = length((gin[0].PosW + gin[1].PosW + gin[2].PosW) / 3 - gEyePosW);
	int lod = DetermineLOD(distance);

	// Subdivide the icosahedron based on the level of detail
	SubdivideIcosahedron(gin[0],gin[1],gin[2], stream, lod);
}

float4 PS(GeometryOut pin) : SV_Target
{
	float4 diffuseAlbedo = gDiffuseMap.Sample(
	 gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
#ifdef ALPHA_TEST

	// Discard pixel if texture alpha < 0.1. We do this test as soon
	// as possible in the shader so that we can potentially exit the
	// shader early, thereby skipping the rest of the shader code.
	clip(diffuseAlbedo.a - 0.1f);

#endif
	// Interpolating normal can unnormalize it, so renormalize it.
	pin.NormalW = normalize(pin.NormalW);
	// Vector from point being lit to eye.
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize
	// Light terms.
	float4 ambient = gAmbientLight*diffuseAlbedo;
	const float shininess = 1.0f - gRoughness;

	Material mat = { diffuseAlbedo, gFresnelR0, shininess };
	float3 shadowFactor = 1.0f;
	float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
	pin.NormalW, toEyeW, shadowFactor);
	float4 litColor = ambient + directLight;
#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif
	// Common convention to take alpha from diffuse albedo.
	litColor.a = diffuseAlbedo.a;
	return litColor;
}


