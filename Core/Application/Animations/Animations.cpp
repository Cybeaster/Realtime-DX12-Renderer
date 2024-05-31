
#include "Animations.h"

#include "Engine/Engine.h"
#include "Logger.h"
#include "MathUtils.h"

STransform OAnimation::PerfomAnimation(const float DeltaTime)
{
	using namespace Utils::Math;
	const auto currentAnimPos = Frames[CurrentIndex].Transform.Position;
	const auto currentAnimRot = DegreesToRadians(Frames[CurrentIndex].Transform.Rotation);
	const auto currentAnimScale = Frames[CurrentIndex].Transform.Scale;

	const auto startAnimPos = StartTransform.Position;
	const auto startAnimRot = StartTransform.Rotation;
	const auto startAnimScale = StartTransform.Scale;
	const auto duration = Frames[CurrentIndex].Duration;

	ElapsedTime += DeltaTime;
	bool hasTimedOut = ElapsedTime >= duration;
	float interpolationFactor = hasTimedOut ? 1 : (ElapsedTime / duration);

	const auto newRot = XMQuaternionSlerp(startAnimRot,
	                                      XMQuaternionRotationRollPitchYawFromVector(currentAnimRot),
	                                      interpolationFactor);

	const auto newPos = XMVectorLerp(startAnimPos, currentAnimPos, interpolationFactor);
	const auto newScale = XMVectorLerp(startAnimScale, currentAnimScale, interpolationFactor);
	if (hasTimedOut)
	{
		StartTransform = Frames[CurrentIndex].Transform;
		StartTransform.Rotation = newRot;
		CurrentIndex++;
		ElapsedTime = 0.0f;
	}

	auto float3Pos = XMFLOAT3();
	Put(float3Pos, newPos);

	auto float4Rot = XMFLOAT4();
	Put(float4Rot, newRot);

	auto float3Scale = XMFLOAT3();
	Put(float3Scale, newScale);

	OEngine::Get()->DrawDebugBox(float3Pos, { 100, 100, 100 }, { 1, 0, 0, 1 }, SColor::Red, 5);

	return STransform{ float3Pos, float4Rot, float3Scale };
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