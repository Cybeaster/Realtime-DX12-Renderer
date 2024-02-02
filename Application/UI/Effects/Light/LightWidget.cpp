
#include "LightWidget.h"

#include "Engine/Engine.h"

#include <ranges>

void OLightWidget::Draw()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::SeparatorText("Ambient Light");
		ImGui::ColorEdit3("Ambient Color", &AmbientColor.x);

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
			if (SelectedLightIdx < 0)
			{
				SelectedLightIdx = 0;
			}
		}

		if (ImGui::BeginListBox("Light list"))
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
void OLightWidget::Init()
{
	SWidgetLight Light1;
	Light1.Light.Direction = { 0.57735f, -0.57735f, 0.57735f };
	Light1.Light.Strength = { 0.6f, 0.6f, 0.6f };
	Light1.Name = "Light 0";
	Light1.Index = 0;

	SWidgetLight light2;
	light2.Light.Direction = { -0.57735f, -0.57735f, 0.57735f };
	light2.Light.Strength = { 0.3f, 0.3f, 0.3f };
	light2.Name = "Light 1";
	light2.Index = 1;

	SWidgetLight light3;
	light3.Light.Direction = { 0.0f, -0.707f, -0.707f };
	light3.Light.Strength = { 0.15f, 0.15f, 0.15f };
	light3.Name = "Light 2";
	light3.Index = 2;
	Lights[0] = Light1;
	Lights[1] = light2;
	Lights[2] = light3;
	SelectedLightIdx = 0;
}

void OLightWidget::Update()
{
	IWidget::Update();

	Engine->SetAmbientLight(AmbientColor);
	vector<SLight> Lights;
	Lights.reserve(Lights.size());
	for (auto& val : this->Lights | std::views::values)
	{
		Lights.push_back(val.Light);
	}
	Engine->SetLightSources(Lights);
}