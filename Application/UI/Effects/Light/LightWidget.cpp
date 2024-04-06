
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
}

void OLightWidget::Update()
{
	IWidget::Update();

	Engine->SetAmbientLight(AmbientColor);
}