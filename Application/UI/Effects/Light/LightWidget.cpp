
#include "LightWidget.h"

#include "Engine/Engine.h"

#include <ranges>

void OLightWidget::Draw()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::Checkbox("Enable Light", &bEnable);

		ImGui::SeparatorText("Ambient Light");
		ImGui::ColorEdit4("Ambient Color", &AmbientColor.x);

		ImGui::SeparatorText("Light List");

		if (ImGui::Button("Add Light"))
		{
			SelectedLightIdx += 1;
			SWidgetLight LightWidget;
			LightWidget.Index = SelectedLightIdx;
			LightWidget.Name = "Light " + std::to_string(SelectedLightIdx);

			Lights[SelectedLightIdx] = LightWidget;
		}

		if (ImGui::Button("Remove Light"))
		{
			Lights[SelectedLightIdx] = {};
			SelectedLightIdx -= 1;
		}

		if (ImGui::BeginListBox("Light list"))
		{
			if (SelectedLightIdx >= 0)
			{
				for (auto& val : Lights | std::views::values)
				{
					if (val.Index != -1)
					{
						if (ImGui::Selectable(val.Name.c_str(), SelectedLightIdx == val.Index))
						{
							SelectedLightIdx = val.Index;
						}
					}
				}
			}
			ImGui::EndListBox();
		}

		if (SelectedLightIdx != -1)
		{
			SLight& Light = Lights[SelectedLightIdx].Light;
			ImGui::InputText("Light Name", LightBuffer, 256);
			ImGui::SliderFloat3("Light Position", &Light.Position.x, -1000.0f, 1000.0f);
			ImGui::SliderFloat3("Light Direction", &Light.Direction.x, -1.0f, 1.0f);
			ImGui::SliderFloat3("Light Strength", &Light.Strength.x, 0.0f, 1.0f);
			ImGui::SliderFloat("Light Spot Power", &Light.SpotPower, 0.0f, 100.0f);
			ImGui::SliderFloat("Light FallOff Start", &Light.FallOffStart, 0.0f, 100.0f);
			ImGui::SliderFloat("Light FallOff End", &Light.FallOffEnd, 0.0f, 100.0f);
		}
	}
}

void OLightWidget::Update()
{
	IWidget::Update();

	if (bEnable)
	{
		Engine->SetAmbientLight(AmbientColor);
		vector<SLight> Lights;
		Lights.reserve(Lights.size());
		for (auto& val : this->Lights | std::views::values)
		{
			Lights.push_back(val.Light);
		}
		Engine->SetLightSources(Lights);
	}
	else
	{
		Engine->SetAmbientLight({ 0.0f, 0.0f, 0.0f, 0.0f });
		Engine->SetLightSources({});
	}
}