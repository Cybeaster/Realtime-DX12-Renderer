
#define ZERO_FLOAT3 float3(0.0f, 0.0f, 0.0f)
#define ONE_FLOAT3 float3(1.0f, 1.0f, 1.0f)
#define ZERO_FLOAT4 float4(0.0f, 0.0f, 0.0f, 0.0f)
#define ONE_FLOAT4 float4(1.0f, 1.0f, 1.0f, 1.0f)
#define FLOAT3(x) float3(x, x, x)
#define FLOAT4X4(x) float4x4(x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x)
#define INV_PI 0.31830988618f
#define PI 3.14159265359f
#define MEDIUMP_FLT_MAX    65504.0
#define SaturateMediumUp(x) min(x, MEDIUMP_FLT_MAX)
#define SQUARE(x) (x * x)

#define HLSL
#include "../Types/DirectX/HlslTypes.h"

void Swap(inout float A, inout float B)
{
	float temp = A;
	A = B;
	B = temp;
}

// ACES Tone Mapping matrices and functions
static const float3x3 ACESInputMat = {
	float3(0.59719, 0.35458, 0.04823),
	float3(0.07600, 0.90834, 0.01566),
	float3(0.02840, 0.13383, 0.83777)
};


static const float3x3 ACESOutputMatrix = {
	float3(1.60475, -0.53108, -0.07367),
	float3(-0.10208, 1.10813, -0.00605),
	float3(-0.00327, -0.07276, 1.07602)
};

float3 RttAndOdtFit(float3 v) {
	float3 a = v * (v + 0.0245786) - 0.000090537;
	float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return a / b;
}

float3 AcesFitted(float3 v) {
	v = mul(ACESInputMat, v);
	v = RttAndOdtFit(v);
	return mul(ACESOutputMatrix, v);
}