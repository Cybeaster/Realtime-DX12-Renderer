
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
				auto& dirLight = LightComponent->GetDirectionalLight();
				dirty |= ImGui::InputFloat3("Direction", &dirLight.Direction.x);
				dirty |=ImGui::ColorEdit3("Color", &dirLight.Strength.x);
				break;
			}
			case ELightType::Point:
			{
				auto& pointLight= LightComponent->GetPointLight();
				dirty |=ImGui::ColorEdit3("Color", &pointLight.Strength.x);
				dirty |=ImGui::InputFloat("Falloff Start", &pointLight.FallOffStart, 0.1f);
				dirty |=ImGui::InputFloat("Falloff End", &pointLight.FallOffEnd, 0.1f);
				break;

			}
			case ELightType::Spot:
			{
				auto& spotLight = LightComponent->GetSpotLight();
				dirty |=ImGui::InputFloat3("Direction", &spotLight.Direction.x);
				dirty |=ImGui::ColorEdit3("Color", &spotLight.Strength.x);
				dirty |=ImGui::InputFloat("Falloff Start", &spotLight.FallOffStart, 0.1f);
				dirty |=ImGui::InputFloat("Falloff End", &spotLight.FallOffEnd, 0.1f);
				dirty |=ImGui::InputFloat("Spot Power", &spotLight.SpotPower, 0.1f);
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
