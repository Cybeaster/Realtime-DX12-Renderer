#include <HighResolutionClock.h>

OHighResolutionClock::OHighResolutionClock()
    : DeltaTime(0)
    , TotalTime(0)
{
	T0 = std::chrono::high_resolution_clock::now();
}

void OHighResolutionClock::Tick()
{
	const auto t1 = std::chrono::high_resolution_clock::now();
	DeltaTime = t1 - T0;
	TotalTime += DeltaTime;
	T0 = t1;
}

void OHighResolutionClock::Reset()
{
	T0 = std::chrono::high_resolution_clock::now();
	DeltaTime = std::chrono::high_resolution_clock::duration();
	TotalTime = std::chrono::high_resolution_clock::duration();
}

double OHighResolutionClock::GetDeltaNanoseconds() const
{
	return DeltaTime.count() * 1.0;
}
double OHighResolutionClock::GetDeltaMicroseconds() const
{
	return DeltaTime.count() * 1e-3;
}

double OHighResolutionClock::GetDeltaMilliseconds() const
{
	return DeltaTime.count() * 1e-6;
}

double OHighResolutionClock::GetDeltaSeconds() const
{
	return DeltaTime.count() * 1e-9;
}

double OHighResolutionClock::GetTotalNanoseconds() const
{
	return TotalTime.count() * 1.0;
}

double OHighResolutionClock::GetTotalMicroseconds() const
{
	return TotalTime.count() * 1e-3;
}

double OHighResolutionClock::GetTotalMilliSeconds() const
{
	return TotalTime.count() * 1e-6;
}

double OHighResolutionClock::GetTotalSeconds() const
{
	return TotalTime.count() * 1e-9;
}
