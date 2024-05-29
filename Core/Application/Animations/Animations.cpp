
#include "Animations.h"

#include "Engine/Engine.h"
#include "Logger.h"
#include "MathUtils.h"

STransform OAnimation::PerfomAnimation(const float DeltaTime)
{
	using namespace Utils::Math;
	const auto currentAnimPos = Load(Frames[CurrentIndex].Transform.Position);
	const auto currentAnimRot = Load(Frames[CurrentIndex].Transform.Rotation);
	const auto currentAnimScale = Load(Frames[CurrentIndex].Transform.Scale);
	const auto duration = Frames[CurrentIndex].Duration;
	ElapsedTime += DeltaTime;
	float interpolationFactor = (ElapsedTime / duration);
	const bool hasEndedFrame = ElapsedTime >= duration;
	if (hasEndedFrame)
	{
		interpolationFactor = 1;
		ElapsedTime = 0.0f;
		StartTransform = Frames[CurrentIndex].Transform;
		CurrentIndex++;
	}
	const auto newPos = XMVectorLerp(Load(StartTransform.Position), currentAnimPos, interpolationFactor);
	const auto newRot = XMVectorLerp(Load(StartTransform.Rotation), currentAnimRot, interpolationFactor);
	const auto newScale = XMVectorLerp(Load(StartTransform.Scale), currentAnimScale, interpolationFactor);
	auto float3Pos = XMFLOAT3();
	Put(float3Pos, newPos);

	auto float3Rot = XMFLOAT3();
	Put(float3Rot, newRot);

	auto float3Scale = XMFLOAT3();
	Put(float3Scale, newScale);

	OEngine::Get()->DrawDebugBox(float3Pos, { 100, 100, 100 }, { 1, 0, 0, 1 }, SColor::Red, 5);

	return { float3Pos, float3Rot, float3Scale };
}

bool OAnimation::IsFinished() const
{
	return CurrentIndex == Frames.size();
}

void OAnimation::SetName(const wstring& NewName)
{
	Name = NewName;
}

const wstring& OAnimation::GetName() const
{
	return Name;
}

void OAnimation::SetFrames(const vector<SAnimationFrame>& NewFrames)
{
	Frames = NewFrames;
}

void OAnimation::SetFrames(vector<SAnimationFrame>&& NewFrames)
{
	Frames = std::move(NewFrames);
}

vector<SAnimationFrame>& OAnimation::GetFrames()
{
	return Frames;
}

void OAnimation::StartAnimation(const STransform& InCurrentTransform)
{
	StartTransform = InCurrentTransform;
	bIsPlaying = true;
	CurrentIndex = 0;
	ElapsedTime = 0.0f;
}

void OAnimation::StopAnimation()
{
	bIsPlaying = false;
	CurrentIndex = 0;
	ElapsedTime = 0.0f;
}

void OAnimation::PauseAnimation()
{
	bIsPlaying = false;
}

bool OAnimation::IsPlaying() const
{
	return bIsPlaying;
}