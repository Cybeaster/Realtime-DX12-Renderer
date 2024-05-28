
#include "Animations.h"

#include "Engine/Engine.h"
#include "Logger.h"
#include "MathUtils.h"

STransform OAnimation::PerfomAnimation(const STransform& Current, const float DeltaTime)
{
	using namespace Utils::Math;
	auto eps = XMVectorReplicate(EPSILON);
	const auto currentAnimPos = Load(Frames[CurrentIndex].Transform.Position);
	const auto currentAnimRot = Load(Frames[CurrentIndex].Transform.Rotation);
	const auto currentAnimScale = Load(Frames[CurrentIndex].Transform.Scale);
	const auto time = Frames[CurrentIndex].Time;
	const auto duration = Frames[CurrentIndex].Duration;
	ElapsedTime += DeltaTime;
	float interpolationFactor = ElapsedTime / duration;
	if (interpolationFactor > 1.0f)
	{
		interpolationFactor = 1.0f; // Clamp to 1.0 if it exceeds
	}
	auto newPos = XMVectorLerp(Load(Current.Position), currentAnimPos, interpolationFactor);
	auto posDiff = Abs(XMVectorSubtract(currentAnimPos, newPos));

	auto newRot = XMVectorLerp(Load(Current.Rotation), currentAnimRot, interpolationFactor);
	auto rotDiff = Abs(XMVectorSubtract(currentAnimRot, newRot));
	auto newScale = XMVectorLerp(Load(Current.Scale), currentAnimScale, interpolationFactor);
	auto scaleDiff = XMVectorAbs(XMVectorSubtract(currentAnimScale, newScale));

	auto float3Pos = XMFLOAT3();
	Put(float3Pos, currentAnimPos);

	auto float3Rot = XMFLOAT3();
	Put(float3Rot, currentAnimRot);

	auto float3Scale = XMFLOAT3();
	Put(float3Scale, currentAnimScale);

	if (XMVector3Greater(eps, posDiff) && XMVector3Greater(eps, rotDiff) && XMVector3Greater(eps, scaleDiff))
	{
		CurrentIndex++;
		LOG(Engine, Log, "Animation is advanced to the next position");

		OEngine::Get()->DrawDebugBox(float3Pos, { 10, 10, 10 }, { 1, 0, 0, 1 }, SColor::Red, 3);
	}
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