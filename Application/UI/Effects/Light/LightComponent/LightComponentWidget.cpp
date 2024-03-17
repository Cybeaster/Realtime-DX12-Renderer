
#include "LightComponentWidget.h"

#include "LightComponent/LightComponent.h"

void OLightComponentWidget::Draw()
{
	ImGui::SeparatorText("Light SelectedComponent");
	if (LightComponent)
	{
		ImGui::Text("Light Type: %s", ToString(LightComponent->GetLightType()).c_str());
		bool dirty = false;
		switch (LightComponent->GetLightType())
		{
			case ELightType::Directional:
			{
				auto& [Direction, Strength] = LightComponent->GetDirectionalLight();
				dirty |= ImGui::DragFloat3("Direction", &Direction.x, 0.1f);
				dirty |=ImGui::ColorEdit3("Color", &Strength.x);
				break;
			}
			case ELightType::Point:
			{
				auto& [Position, Strength, FallOffStart, FallOffEnd] = LightComponent->GetPointLight();
				dirty |=ImGui::DragFloat3("Position", &Position.x, 0.1f);
				dirty |=ImGui::ColorEdit3("Color", &Strength.x);
				dirty |=ImGui::DragFloat("Falloff Start", &FallOffStart, 0.1f);
				dirty |=ImGui::DragFloat("Falloff End", &FallOffEnd, 0.1f);
				break;

			}
			case ELightType::Spot:
			{
				auto& [Position, Direction, Strength, FallOffStart, FallOffEnd, SpotPower] = LightComponent->GetSpotLight();
				dirty |=ImGui::DragFloat3("Position", &Position.x, 0.1f);
				dirty |=ImGui::DragFloat3("Direction", &Direction.x, 0.1f);
				dirty |=ImGui::ColorEdit3("Color", &Strength.x);
				dirty |=ImGui::DragFloat("Falloff Start", &FallOffStart, 0.1f);
				dirty |=ImGui::DragFloat("Falloff End", &FallOffEnd, 0.1f);
				dirty |=ImGui::DragFloat("Spot Power", &SpotPower, 0.1f);
				break;
			}
		}

		if (dirty)
		{
			LightComponent->MarkDirty();
		}
	}
	else
	{
		ImGui::Text("No Light SelectedComponent");
	}
}

void OLightComponentWidget::SetComponent(OLightComponent* InComponent)
{
	LightComponent = InComponent;
}
