#include "AnimationListWidget.h"

#include "Animations/AnimationManager.h"
#include "Animations/Animations.h"
#include "DirectX/DXHelper.h"

void OAnimationListWidget::DrawTable()
{
	const auto& animations = AnimationManager.lock()->GetAllAnimations();
	if (ImGui::Button("Save Animation"))
	{
		AnimationManager.lock()->SaveAnimations();
	}

	for (const auto& [name, anim] : animations)
	{
		auto utfName = WStringToUTF8(name);
		if (ImGui::Selectable(utfName.c_str()))
		{
			CurrentAnimation = anim;
		}
	}
}

void OAnimationListWidget::DrawProperty()
{
	if (!CurrentAnimation.expired())
	{
		auto lock = CurrentAnimation.lock();
		ImGui::Text(WStringToUTF8(lock->GetName()).c_str());
		ImGui::SeparatorText("Animation Frames");
		auto& frames = lock->GetFrames();
		for (size_t i = 0; i < frames.size(); i++)
		{
			ImGui::Text("Frame %d", i);
			ImGui::InputFloat3("Translation", &frames[i].Transform.Position.x);
			ImGui::InputFloat3("Rotation", &frames[i].Transform.Rotation.x);
			ImGui::InputFloat3("Scale", &frames[i].Transform.Scale.x);
		}
	}
	else
	{
		ImGui::Text("No animation selected");
	}
}