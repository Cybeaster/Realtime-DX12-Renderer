#include "AnimationManager.h"

#include "Animations.h"
#include "AnimationsReader/AnimationsReader.h"
#include "Application.h"

OAnimationManager::OAnimationManager()
{
	AnimationsReader = make_unique<OAnimationsReader>(OApplication::Get()->GetConfigPath("AnimationsConfigPath"));
	LoadAnimations();
}
OAnimationManager::~OAnimationManager()
{
}

const OAnimationManager::TAnimations& OAnimationManager::GetAllAnimations() const
{
	return Animations;
}

void OAnimationManager::SaveAnimations()
{
	vector<shared_ptr<OAnimation>> animations;
	for (const auto& [name, anim] : Animations)
	{
		animations.push_back(anim);
	}
	AnimationsReader->SaveAnimations(animations);
}

void OAnimationManager::LoadAnimations()
{
	Animations.clear();
	auto animations = AnimationsReader->ReadAnimations();
	for (const auto& anim : animations)
	{
		Animations[anim->GetName()] = anim;
	}
}