
#include "LightWidget.h"

#include "Engine/Engine.h"

#include <ranges>

void OLightWidget::Draw()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::SeparatorText("Ambient Light");
		ImGui::ColorEdit4("Ambient Color", &AmbientColor.x);

	}
}
void OLightWidget::InitWidget()
{
	/*SWidgetLight Light1;
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
	SelectedLightIdx = 0;*/
}

void OLightWidget::Update()
{
	IWidget::Update();

	Engine->SetAmbientLight(AmbientColor);
}