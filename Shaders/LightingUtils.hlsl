
#define MAX_LIGHTS 16

struct Light
{
	float3 Strength ; //Light Color
	float FallOffStart ; // point/spot light only

	float3 Direction; // directional/spot light only
	float FallOffEnd; // point/spot light only

	float3 Position; // point/spot light only
	float SpotPower; // spot light only
};

struct Material
{
	float4 DiffuseAlbedo ;
	float3 FresnelR0 ;
	float Shininess;
};


float CalcAttenuation(float Distance, float FallOffStart, float FallOffEnd)
{
	return saturate((FallOffEnd - Distance) / (FallOffEnd - FallOffStart));
}

// Schlick gives an approximation to Fresnel reflectance
// (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.


float3 SchlickFresnel(float3 R0, float3 Normal, float3 LightDir)
{
	float cosIncidentAngle = saturate(dot(Normal, LightDir));
	float f0  = 1.0 - cosIncidentAngle;
	float3 reflectedPercent = R0 + (1.0 - R0) * pow(f0, 5.0);
	return reflectedPercent;
}


float3 BlinnPhong(float3 LightStrength,float3 LightDir, float3 Normal, float3 EyeDir, Material Mat)
{
	// Derive m from the shininess, which is derived from the roughness.
	const float m = Mat.Shininess * 256.0f;
	float3 halfVec = normalize(LightDir + EyeDir);
	float roughnessFactor = (m+8.0f)*pow(max(dot(halfVec, Normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = SchlickFresnel(Mat.FresnelR0, Normal, EyeDir);

	// Our spec formula goes outside [0,1] range, but we are doing
	// LDR rendering. So scale it down a bit.
	float3 specAlbedo = fresnelFactor * roughnessFactor;
	specAlbedo /= (specAlbedo + 1.0f);
	return Mat.DiffuseAlbedo.rgb + specAlbedo * LightStrength;
}

float3 ComputeDirectionalLight(Light L, Material Mat, float3 Normal, float3 EyeDir)
{
	float3 LightVec = -L.Direction;
	// Scale light down by Lambert’s cosine law
	float nDotL = max(dot(Normal, LightVec), 0.0f);
	float3 lightStrength = nDotL * L.Strength;
	return BlinnPhong(lightStrength, LightVec, Normal, EyeDir, Mat);
}

float3 ComputePointLight(Light L, Material Mat, float3 Pos, float3 Normal, float3 EyeDir)
{
	// The vector from the surface to the light.
	float3 lightVec = L.Position - Pos;

	// The distance from surface to light.
	float dist = length(lightVec);

	if(dist > L.FallOffEnd)
		return 0.f;

	// Normalize the light vector.
	lightVec /= dist;

	// Scale light down by Lambert’s cosine law
	float nDotL = max(dot(Normal, lightVec), 0.0f);
	float3 lightStrength = nDotL * L.Strength;

	// Attenuate light based on distance.
	float attenuation = CalcAttenuation(dist, L.FallOffStart, L.FallOffEnd);
	lightStrength *= attenuation;
	return BlinnPhong(lightStrength, lightVec, Normal, EyeDir, Mat);
}

float3 ComputeSpotLight(Light L, Material Mat, float3 Pos, float3 Normal, float3 ToEye)
{
	// The vector from the surface to the light.
	float3 lightVec = L.Position - Pos;

	// The distance from surface to light.
	float dist = length(lightVec);

	if(dist > L.FallOffEnd)
		return 0.f;

	// Normalize the light vector.
	lightVec /= dist;

	// Scale light down by Lambert’s cosine law
	float nDotL = max(dot(Normal, lightVec), 0.0f);
	float3 lightStrength = nDotL * L.Strength;

	// Attenuate light based on distance.
	float attenuation = CalcAttenuation(dist, L.FallOffStart, L.FallOffEnd);
	lightStrength *= attenuation;

	// Attenuate light by distance.
	float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
	lightStrength *= spotFactor;

	return BlinnPhong(lightStrength, lightVec, Normal, ToEye, Mat);
}

float4 ComputeLighting(Light Lights[MAX_LIGHTS], Material Mat, float3 Pos, float3 Normal, float3 ToEye, float3 ShadowFactor)
{
	float3 result = 0.0f;
	int i =0;

#if (NUM_DIR_LIGHTS > 0)
	for(i = 0; i < NUM_DIR_LIGHTS; ++i)
	{
		result += ShadowFactor[i] * ComputeDirectionalLight(Lights[i], Mat, Normal, ToEye);
	}
#endif

#if (NUM_POINT_LIGHTS > 0)
	for(; i < NUM_POINT_LIGHTS; ++i)
	{
		result += ComputePointLight(Lights[i], Mat, Pos, Normal, ToEye);
	}
#endif

#if (NUM_SPOT_LIGHTS > 0)
	for(; i < NUM_SPOT_LIGHTS; ++i)
	{
		result +=  ComputeSpotLight(Lights[i], Mat, Pos, Normal, ToEye);
	}
#endif

return float4(result,0.0f);
}
