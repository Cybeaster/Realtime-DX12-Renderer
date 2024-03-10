//=============================================================================
// Performs a separable Guassian blur with a blur radius up to 5 pixels.
//=============================================================================

#define N 256
static const int MaxBlurRadius = 5;
#define CacheSize (N + 2 * MaxBlurRadius)


Texture2D Input : register(t0);
RWTexture2D<float4> Output : register(u0);

cbuffer cbSettings : register(b0)
{
	// We cannot have an array entry in a constant buffer that gets mapped onto
	// root constants, so list each element.

	int BlurRadius;

	// Support up to 11 blur weights.
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

groupshared float4 Cache[CacheSize];

[numthreads(N, 1, 1)] void HorzBlurCS(int3 GroupThreadID
                                      : SV_GroupThreadID,
                                        int3 DispatchThreadID
                                      : SV_DispatchThreadID) {
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels,
	// have 2*BlurRadius threads sample an extra pixel.
	if (GroupThreadID.x < BlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(DispatchThreadID.x - BlurRadius, 0);
		Cache[GroupThreadID.x] = Input[int2(x, DispatchThreadID.y)];
	}
	uint width, height;
	Input.GetDimensions(width, height);

	if (GroupThreadID.x >= N - BlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(DispatchThreadID.x + BlurRadius, width - 1);
		Cache[GroupThreadID.x + 2 * BlurRadius] = Input[int2(x, DispatchThreadID.y)];
	}

	Cache[GroupThreadID.x + BlurRadius] = Input[min(DispatchThreadID.xy, uint2(width,height) - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);
	for (int i = -BlurRadius; i <= BlurRadius; ++i)
	{
		int k = GroupThreadID.x + BlurRadius + i;
		blurColor += weights[i + BlurRadius] * Cache[k];
	}
	Output[DispatchThreadID.xy] = blurColor;
}

    [numthreads(1, N, 1)] void VertBlurCS(int3 GroupThreadID
                                          : SV_GroupThreadID,
                                            int3 DispatchThreadID
                                          : SV_DispatchThreadID)
{
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels,
	// have 2*BlurRadius threads sample an extra pixel.
	if (GroupThreadID.y < BlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(DispatchThreadID.y - BlurRadius, 0);
		Cache[GroupThreadID.y] = Input[int2(DispatchThreadID.x, y)];
	}

	uint width, height;
	Input.GetDimensions(width, height);

	if (GroupThreadID.y >= N - BlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(DispatchThreadID.y + BlurRadius, height - 1);
		Cache[GroupThreadID.y + 2 * BlurRadius] = Input[int2(DispatchThreadID.x, y)];
	}

	// Clamp out of bound samples that occur at image borders.
	Cache[GroupThreadID.y + BlurRadius] = Input[min(DispatchThreadID.xy, uint2(width,height) - 1)];
	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	float4 blurColor = float4(0, 0, 0, 0);
	for (int i = -BlurRadius; i <= BlurRadius; ++i)
	{
		int k = GroupThreadID.y + BlurRadius + i;
		blurColor += weights[i + BlurRadius] * Cache[k];
	}
	Output[DispatchThreadID.xy] = blurColor;
}