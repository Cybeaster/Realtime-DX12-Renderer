#include "Types.hlsl"

//Specular term D, aka NDF
float D_GGX(float NoH, float Roughness, float3 N, float3 H)
{
	float3 NxH = cross(N, H);
	float a = NoH* Roughness;
	float k = Roughness / (dot(NxH, NxH) + a*a);
	float d = k*k*INV_PI;
	return SaturateMediumUp(d);
}

float D_GGX(float NoH, float Roughness)
{
	float a2 = Roughness * Roughness;
	float f = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * f * f);
}

//Geometric shadowing aka specular G
float V_SmithGGXCorrelated(float NoV, float NoL, float Roughness)
{
	float a2 = Roughness * Roughness;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5f / (GGXV + GGXL);
}

float3 F_Schlick(float U, float3 F0, float f90)
{
	return F0 + (FLOAT3(f90) - F0) * pow(1.0 - U, 5.0);
}

float3 F_Schlick(float U, float3 F0)
{
	  return F0 + (FLOAT3(1.0) - F0) * pow(1.0 - U, 5.0);
}

float F_Schlick(float U, float F0, float F90)
{
	return F0 + (F90 - F0) * pow(1.0 - U, 5.0);
}


float Fd_Burley(float NoV, float NoL, float LoH, float Roughness)
{
	float f90 = 0.5 + 2 * Roughness * SQUARE(LoH);
	float lightScatter = F_Schlick(NoL, 1.0, f90);
	float viewScatter = F_Schlick(NoV, 1.0, f90);
	return lightScatter * viewScatter * INV_PI;
}

float3 EnergyCompensation(float F0, float Y)
{
	return 1 + F0 * (1 / Y - 1);
}

float Fd_Lambert()
{
	return INV_PI;
}

float V_Kelemen(float LoH)
{
	return 0.25 / (LoH * LoH);
}