
#define ZERO_FLOAT3 float3(0.0f, 0.0f, 0.0f)
#define ONE_FLOAT3 float3(1.0f, 1.0f, 1.0f)
#define ZERO_FLOAT4 float4(0.0f, 0.0f, 0.0f, 0.0f)
#define ONE_FLOAT4 float4(1.0f, 1.0f, 1.0f, 1.0f)
#define FLOAT3(x) float3(x, x, x)
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

