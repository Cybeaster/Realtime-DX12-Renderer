#include "AnimationListWidget.h"

#include "Animations/AnimationManager.h"
#include "Animations/Animations.h"
#include "DirectX/DXHelper.h"

void OAnimationListWidget::DrawTable()
{
	const auto& animations = AnimationManager->GetAllAnimations();
	if (ImGui::Button("Save Animation"))
	{
		AnimationManager->SaveAnimations();
	}

	if (ImGui::Button("Reload Animation"))
	{
		AnimationManager->LoadAnimations();
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
			auto translationText = "%%Translation##" + std::to_string(i);
			auto rotationText = "Rotation##" + std::to_string(i);
			auto scaleText = "Scale##" + std::to_string(i);
			auto durationText = "Duration##" + std::to_string(i);
			ImGui::InputFloat3(translationText.c_str(), &frames[i].Transform.Position.x);
			ImGui::InputFloat3(rotationText.c_str(), &frames[i].Transform.Rotation.x);
			ImGui::InputFloat3(scaleText.c_str(), &frames[i].Transform.Scale.x);
			ImGui::InputFloat(durationText.c_str(), &frames[i].Duration);
		}
	}
	else
	{
		ImGui::Text("No animation selected");
	}
}