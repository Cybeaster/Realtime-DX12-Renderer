
cbuffer cbSsao : register(b0)
{
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gProjTex;
	float4   gOffsetVectors[14];
    float4 gBlurWeights[3];
    float2 gInvRenderTargetSize;

    // Coordinates given in view space.
    float    gOcclusionRadius;
    float    gOcclusionFadeStart;
    float    gOcclusionFadeEnd;
    float    gSurfaceEpsilon;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalMap    : register(t0);
Texture2D gDepthMap     : register(t1);
Texture2D gRandomVecMap : register(t2);

SamplerState gsamPointClamp : register(s0);
SamplerState gsamLinearClamp : register(s1);
SamplerState gsamDepthMap : register(s2);
SamplerState gsamLinearWrap : register(s3);
static const int gSampleCount = 14;



static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
	float2 TexC : TEXCOORD0;
};

VertexOut VS(uint vid : SV_VertexID)
{
 VertexOut vout;

     vout.TexC = gTexCoords[vid];

     // Quad covering screen in NDC space.
     vout.PosH = float4(2.0f*vout.TexC.x - 1.0f, 1.0f - 2.0f*vout.TexC.y, 0.0f, 1.0f);

     // Transform quad corners to view space near plane.
     float4 ph = mul(vout.PosH, gInvProj);
     vout.PosV = ph.xyz / ph.w;

     return vout;
}

// Determines how much the sample point q occludes the point p as a function
// of distZ.

float OcclusionFunction(float DistZ)
{
    float occlusion = 0.0f;
    if(DistZ > gOcclusionRadius)
    {
        float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
        occlusion = saturate((gOcclusionFadeEnd-DistZ)/fadeLength);
    }
    return occlusion;
}

float NdcDepthToViewDepth(float z_ndc)
{
    float viewZ = gProj[3][2] / (z_ndc - gProj[2][2]);
    return viewZ;
}

float4 PS(VertexOut Pin ) : SV_Target
{
    	// p -- the point we are computing the ambient occlusion for.
    	// n -- normal vector at p.
    	// q -- a random offset from p.
    	// r -- a potential occluder that might occlude p.
    	float3 n = normalize(gNormalMap.SampleLevel(gsamPointClamp, Pin.TexC, 0).xyz);
    	float pz = gDepthMap.SampleLevel(gsamDepthMap, Pin.TexC, 0).r;
    	pz = NdcDepthToViewDepth(pz);

    	float3 p = (pz/Pin.PosV.z)*Pin.PosV;
    	float3 randVec = 2.0f*gRandomVecMap.SampleLevel(gsamLinearWrap, 4.0f*Pin.TexC, 0.0f).rgb - 1.0f;
    	float occlusionSum = 0.0f;
    	for(int i =0; i < gSampleCount; i++)
    	{
    	     //Are offset vectors are fixed and uniformly distributed (so that our offset vectors
           	// do not clump in the same direction).  If we reflect them about a random vector
           	// then we get a random uniform distribution of offset vectors.
           	float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

           	// Flip offset vector if it is behind the plane defined by (p, n).
            float flip = sign( dot(offset, n) );

            // Sample a point near p within the occlusion radius.
            float3 q = p + flip * gOcclusionRadius * offset;

            // Project q and generate projective tex-coords.
		    float4 projQ = mul(float4(q, 1.0f), gProjTex);
		    projQ /= projQ.w;

		    // Find the nearest depth value along the ray from the eye to q (this is not
            // the depth of q, as q is just an arbitrary point near p and might
            		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

            float rz = gDepthMap.SampleLevel(gsamDepthMap, projQ.xy, 0.0f).r;
            rz = NdcDepthToViewDepth(rz);
            float3 r = (rz/q.z)*q;

            float distZ = p.z - r.z;
            float dp = max(dot(n, normalize(r - p)), 0.0f);
            float occlusion = dp*OcclusionFunction(distZ);
            occlusionSum += occlusion;
    	}
    	occlusionSum /= gSampleCount;
        float access = 1.0f - occlusionSum;
        // Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
        return saturate(pow(access, 6.0f));
}