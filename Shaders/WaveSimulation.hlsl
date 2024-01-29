
cbuffer cbUpdateSettings
{
	float WaveConstant0;
	float WaveConstant1;
	float WaveConstant2;

	float DisturbMag;
	int2 DisturbIndex;
};

RWTexture2D<float> PrevSolution : register(u0);
RWTexture2D<float> CurrSolution : register(u1);
RWTexture2D<float> Output : register(u2);

// Solves 2D wave equation using the compute shader.
//clang-format off

[numthreads(16, 16, 1)]
void UpdateWavesCS(int3 DispatchThreadID : SV_DispatchThreadID) {
	// We do not need to do bounds checking because:
	//	 *out-of-bounds reads return 0, which works for us--it just means the boundary of
	//    our water simulation is clamped to 0 in local space.
	//   *out-of-bounds writes are a no-op.

	int x = DispatchThreadID.x;
	int y = DispatchThreadID.y;

	Output[int2(x, y)] =
			 WaveConstant0 * PrevSolution[int2(x, y)].r +
			 WaveConstant1 * CurrSolution[int2(x, y)].r +
			 WaveConstant2 * (
			 		CurrSolution[int2(x + 1, y)].r +
					CurrSolution[int2(x - 1, y)].r +
					CurrSolution[int2(x, y + 1)].r +
					CurrSolution[int2(x, y - 1)].r
			);
}

// Runs one thread to disturb a grid height and its
[numthreads(1, 1, 1)]
void DisturbWavesCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
	int x = DisturbIndex.x;
	int y = DisturbIndex.y;

	float halfMag = 0.5f * DisturbMag;

	Output[int2(x, y)] += DisturbMag;
	Output[int2(x + 1, y)] += halfMag;
	Output[int2(x - 1, y)] += halfMag;
	Output[int2(x, y + 1)] += halfMag;
	Output[int2(x, y - 1)] += halfMag;
}

//clang-format on
