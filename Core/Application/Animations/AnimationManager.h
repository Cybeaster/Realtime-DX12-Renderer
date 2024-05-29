#pragma once
#include "Types.h"

class OAnimation;
class OAnimationsReader;
class OAnimationManager
{
public:
	using TAnimations = unordered_map<wstring, shared_ptr<OAnimation>>;

	OAnimationManager();
	~OAnimationManager();
	const TAnimations& GetAllAnimations() const;
	void SaveAnimations();
	void LoadAnimations();

private:
	unique_ptr<OAnimationsReader> AnimationsReader;
	unordered_map<wstring, shared_ptr<OAnimation>> Animations;
};
