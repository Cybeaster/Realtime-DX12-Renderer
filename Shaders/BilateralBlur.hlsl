#define N 32


cbuffer BilateralBlur : register(b0) {
    float SpatialSigma;
    float IntensitySigma;
    int KernelRadius;
};

cbuffer BufferConstants : register(b1) {
    int TextureWidth;
    int TextureHeight;
}


Texture2D Input : register(t0);
RWTexture2D<float4> Output : register(u0);

[numthreads(N, N, 1)]
void BilateralBlur(uint3 DispatchThreadID : SV_DispatchThreadID) {
    float4 centerPixel = Input[DispatchThreadID.xy];

    float3 colorSum = float3(0,0,0);
    float weightSum = 0;

    // Iterate over the kernel
    for (int x = -KernelRadius; x <= KernelRadius; x++) {
        for (int y = -KernelRadius; y <= KernelRadius; y++) {
            int2 samplePos = DispatchThreadID.xy + int2(x,y);

            // Check if samplePos is within texture bounds
            if (samplePos.x >= 0 && samplePos.y >= 0 && samplePos.x < TextureWidth && samplePos.y < TextureHeight) {
                float4 samplePixel = Input[samplePos];

                // Calculate spatial weight
                float spatialWeight = exp(-((x*x + y*y) / (2 * SpatialSigma * SpatialSigma)));

                // Calculate intensity weight
                float intensityWeight = exp(-length(samplePixel - centerPixel) / (2 * IntensitySigma * IntensitySigma));

                float weight = spatialWeight * intensityWeight;

                colorSum += samplePixel.rgb * weight;
                weightSum += weight;
            }
        }
    }

    if (weightSum > 0) {
        Output[DispatchThreadID.xy] = float4(colorSum / weightSum, 1);
    } else {
        Output[DispatchThreadID.xy] = centerPixel; // Or handle as needed
    }
}
