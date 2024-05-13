
#include "LightComponentWidget.h"

#include "Engine/RenderTarget/CSM/Csm.h"
#include "LightComponent/LightComponent.h"

#include <numeric>

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
			auto component = Cast<ODirectionalLightComponent>(LightComponent);
			auto& dirLight = component->GetDirectionalLight();
			dirty |= ImGui::DragFloat3("Direction", &dirLight.Direction.x, 0.01, -1.0, 1.0);
			dirty |= ImGui::ColorEdit3("Color", &dirLight.Intensity.x);
			auto lambda = component->GetCascadeLambda();
			if (ImGui::DragFloat("Cascade Lambda", &lambda, 0.01f, 0.0f, 1.0f))
			{
				component->SetCascadeLambda(lambda);
			}
			ImGui::DragFloat("Radius Scale", &component->RadiusScale, 0.01f, 0.1f, 10.0f);
			if (ImGui::Begin("Shadow Maps"))
			{
				for (const auto map : component->GetCSM()->GetShadowMaps())
				{
					auto format = std::format("Shadow Map {}", map->GetShadowMapIndex());
					ImGui::SeparatorText(format.c_str());
					auto boxName = std::format("Bounding Box for {}", map->GetShadowMapIndex());
					ImGui::Checkbox(boxName.c_str(), &map->bDrawBoundingGeometry);
					auto numRenderObjects = map->GetCulledInstancesInfo()->InstanceCount;
					auto numTriangles = std::accumulate(map->GetCulledInstancesInfo()->Items.begin(),
					                                    map->GetCulledInstancesInfo()->Items.end(),
					                                    0,
					                                    [](int32_t acc, auto pair) { return acc + pair.first.lock()->ChosenSubmesh.lock()->Vertices->size() / 3; });

					ImGui::Text("Number of rendered meshes: %d", numRenderObjects);
					ImGui::Text("Number of rendered triangles: %d", numTriangles);
					auto typeName = std::format("Bouding volume type: [{}]", WStringToUTF8(TEXT(map->GetBoundingGeometry()->GetType())));
					ImGui::Text(typeName.c_str());
					ImGui::Image(PtrCast(map->GetSRV().GPUHandle.ptr), ImVec2(250, 250));
				}
				ImGui::End();
			}
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
