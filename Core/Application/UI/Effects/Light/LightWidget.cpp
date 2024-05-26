
#include "LightWidget.h"

#include "Engine/Engine.h"

#include <ranges>

void OLightWidget::Draw()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::SeparatorText("Ambient Light");
		ImGui::ColorEdit3("Ambient Color", &AmbientColor.x);
		ImGui::DragFloat("Ambient Intensity", &AmbientColor.w, 0.01f, 0.0f, 1.0f);
	}
}
void OLightWidget::InitWidget()
{
}

void OLightWidget::Update()
{
	IWidget::Update();
	Engine->SetAmbientLight(AmbientColor);
}