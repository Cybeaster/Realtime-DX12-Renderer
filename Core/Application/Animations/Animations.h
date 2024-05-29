#pragma once
#include "Transform.h"
#include "Types.h"

struct SAnimationFrame
{
	STransform Transform;
	float Time = 0.0f;
	float Duration = 0.0f;
};

class OAnimation
{
public:
	OAnimation() = default;

	OAnimation(const wstring& Name, vector<SAnimationFrame>&& Frames)
	    : Name(Name), Frames(std::move(Frames)) {}

	STransform PerfomAnimation(float DeltaTime);
	bool IsFinished() const;

	void SetName(const wstring& NewName);
	const wstring& GetName() const;

	void SetFrames(const vector<SAnimationFrame>& NewFrames);
	void SetFrames(vector<SAnimationFrame>&& NewFrames);
	vector<SAnimationFrame>& GetFrames();
	void StartAnimation(const STransform& InCurrentTransform);
	void StopAnimation();
	void PauseAnimation();
	bool IsPlaying() const;

private:
	STransform StartTransform;
	bool bIsPlaying = false;
	wstring Name;
	vector<SAnimationFrame> Frames;
	uint32_t CurrentIndex = 0;
	float ElapsedTime = 0.0f;
};
