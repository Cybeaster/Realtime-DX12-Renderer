
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
			auto& dirLight = Cast<ODirectionalLightComponent>(LightComponent)->GetDirectionalLight();
			dirty |= ImGui::DragFloat3("Direction", &dirLight.Direction.x, 0.01, -1.0, 1.0);
			dirty |= ImGui::ColorEdit3("Color", &dirLight.Intensity.x);
			break;
		}
		case ELightType::Point:
		{
			auto& pointLight = Cast<OPointLightComponent>(LightComponent)->GetPointLight();
			dirty |= ImGui::ColorEdit3("Color", &pointLight.Intensity.x);
			dirty |= ImGui::InputFloat("Falloff Start", &pointLight.FalloffStart, 0.1f);
			dirty |= ImGui::InputFloat("Falloff End", &pointLight.FalloffEnd, 0.1f);
			break;
		}
		case ELightType::Spot:
		{
			auto& spotLight = Cast<OSpotLightComponent>(LightComponent)->GetSpotLight();
			dirty |= ImGui::DragFloat3("Direction", &spotLight.Direction.x, 0.01, -1.0, 1.0);
			dirty |= ImGui::ColorEdit3("Color", &spotLight.Intensity.x);
			dirty |= ImGui::DragFloat("Falloff Start", &spotLight.FalloffStart, 0.1f, 0.1f, spotLight.FalloffEnd - 0.1f);
			dirty |= ImGui::DragFloat("Falloff End", &spotLight.FalloffEnd, 1.f, spotLight.FalloffStart + 0.1f, 5000.0f);
			dirty |= ImGui::DragFloat("Spot Power", &spotLight.SpotPower, 0.1f, 0.1f, 100.0f);
			dirty |= ImGui::DragFloat("Cone Angle", &spotLight.ConeAngle, 0.1f, 1.0f, 90.0f);
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
