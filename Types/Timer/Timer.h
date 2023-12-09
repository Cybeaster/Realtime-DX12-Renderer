#pragma once
#include <windows.h>

#include <cstdint>
class STimer
{
public:
	STimer();

	float GetTime();
	float GetDeltaTime();

	void Reset();
	void Start();
	void Stop();
	void Tick();

private:
	double SecondsPerCount = 0.0;
	double DeltaTime = -1.0;

	int64_t BaseTime = 0;
	int64_t PausedTime = 0;
	int64_t StopTime = 0;
	int64_t PrevTime = 0;
	int64_t CurrTime = 0;

	bool bIsStopped = false;
};

inline STimer::STimer()
{
	int64_t countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0 / (double)countsPerSec;
}

inline float STimer::GetTime()
{
	if (bIsStopped)
	{
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
	else
	{
		return (float)(((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
	}
}

inline float STimer::GetDeltaTime()
{
	return (float)DeltaTime;
}

inline void STimer::Reset()
{
	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	BaseTime = currTime;
	PrevTime = currTime;
	StopTime = 0;
	bIsStopped = false;
}

inline void STimer::Start()
{
	int64_t startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (bIsStopped)
	{
		PausedTime += (startTime - StopTime);

		PrevTime = startTime;
		StopTime = 0;
		bIsStopped = false;
	}
}
inline void STimer::Stop()
{
	if (!bIsStopped)
	{
		int64_t currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		StopTime = currTime;
		bIsStopped = true;
	}
}

inline void STimer::Tick()
{
	if (bIsStopped)
	{
		DeltaTime = 0.0;
		return;
	}

	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	CurrTime = currTime;

	DeltaTime = (CurrTime - PrevTime) * SecondsPerCount;

	PrevTime = CurrTime;

	if (DeltaTime < 0.0)
	{
		DeltaTime = 0.0;
	}
}