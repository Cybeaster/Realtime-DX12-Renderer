#include "BRDF.hlsl"
float CartoonDiffuse(float Diffuse)
{
	float specularResult = 0.0f;

	if (Diffuse > 0.5)
	{
		Diffuse = 1.0;
	}
	else if (Diffuse > 0.0)
	{
		Diffuse = 0.6;
	}
	else
	{
		Diffuse = 0.4;
	}

	return Diffuse;
}

float CartoonSpecular(float Specular)
{
	if (Specular > 0.8f)
	{
		Specular = 0.8f;
	}
	else if (Specular > 0.1f)
	{
		Specular = 0.5f;
	}
	return Specular;
}

struct Material
{
	float3 DiffuseAlbedo;
	float3 FresnelR0;
	float Shininess;
	float Roughness;
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
	// Linear falloff.
	return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
	float cosIncidentAngle = saturate(dot(normal, lightVec));

	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

	return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
	const float m = mat.Shininess;
	float3 halfVec = normalize(toEye + lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;

#ifdef CARTOON
	roughnessFactor = CartoonSpecular(roughnessFactor);
#endif

	float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	// Our spec formula goes outside [0,1] range, but we are
	// doing LDR rendering.  So scale it down a bit.
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 BRDF(float3 View, float3 Light, float3 Normal, Material Mat)
{
	float3 h = normalize(View + Light);
	float NoV = abs(dot(Normal, View)) + EPSILON;
	float NoL = saturate(dot(Normal, Light));
	float NoH = saturate(dot(Normal, h));
	float LoH = saturate(dot(Light, h));

	float roughness = Mat.Roughness * Mat.Roughness;

	float D = D_GGX(NoH, roughness);
	float3 F = F_Schlick(LoH, Mat.FresnelR0);
	float V = V_SmithGGXCorrelated(NoV, NoL, roughness);

    // specular BRDF
	float3 Fr = (D * V) * F;

    // diffuse BRDF
	float3 Fd =  Mat.DiffuseAlbedo.rgb * Fd_Lambert();
	return ( Fr + Fd ) ;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight_BlinnPhong(DirectionalLight L, Material mat, float3 normal, float3 toEye)
{
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.Direction;

	// Scale light down by Lambert's cosine law.
	float NoL = max(dot(lightVec, normal), 0.0f);

#ifdef CARTOON
	ndotl = CartoonDiffuse(ndotl);
#endif

	float3 lightStrength = L.Intensity * NoL;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeDirectionalLight_BRDF(DirectionalLight L, Material Mat, float3 Normal, float3 ToEye)
{

	float3 lightStrength = L.Intensity * max(dot(-L.Direction, Normal), 0.0f);
	return BRDF(ToEye, -L.Direction, Normal, Mat) * lightStrength * 10 ; //todo move to constant
}

float3 ComputeSpotLight_BRDF(SpotLight L, Material Mat,float3 Position, float3 Normal, float3 ToEye)
{
	float3 lightVec = L.Position - Position;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if (d > L.FalloffEnd)
		return 0.0f;
	lightVec /= d;
	// Scale light down by Lambert's cosine law.
	float NoL = max(dot(lightVec, Normal), 0.0f);
	float3 lightStrength = L.Intensity * NoL;

	// Attenuate light by distance.
	float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
	lightStrength *= att;

	// Scale by spotlight
	float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
	lightStrength *= spotFactor;

	return BRDF(ToEye, lightVec, Normal, Mat);
}


//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight_BlinnPhong(SpotLight L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if (d > L.FalloffEnd)
		return 0.0f;

	// Normalize the light vector.
	lightVec /= d;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = L.Intensity * ndotl;

	// Attenuate light by distance.
	float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
	lightStrength *= att;

	// Scale by spotlight
	float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
	lightStrength *= spotFactor;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(PointLight L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if (d > L.FalloffEnd)
		return 0.0f;

	// Normalize the light vector.
	lightVec /= d;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = L.Intensity * ndotl;

	// Attenuate light by distance.
	float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
	lightStrength *= att;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}


float3 BoxCubeMapLookup(float3 RayOrigin, float3 UnitRayDir, float3 BoxCenter, float3 BoxExtents)
{
	float3 p = RayOrigin - BoxCenter;
	float3 t1 = (-p - BoxExtents) / UnitRayDir;
	float3 t2 = (-p + BoxExtents) / UnitRayDir;
	float3 tmax = max(t1, t2);
	float3 t = min(min(tmax.x, tmax.y), tmax.z);
	return p + t * UnitRayDir;
}


