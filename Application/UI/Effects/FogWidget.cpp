#include "FogWidget.h"

#include "Engine/Engine.h"

void OFogWidget::Draw()
{
	if (ImGui::CollapsingHeader("Fog"))
	{
		ImGui::Checkbox("Is Fog Enabled", &bEnabled);
		ImGui::SliderFloat("Fog Start", &FogStart, 0.0f, 200.0f);
		ImGui::SliderFloat("Fog Range", &FogRange, 0.0f, 200.0f);
		ImGui::ColorEdit3("Fog Color", &FogColor.x, ImGuiColorEditFlags_Float);
	}
}

void OFogWidget::Update()
{
	if (bEnabled)
	{
		if (Engine == nullptr)
		{
			LOG(Widget, Error, "Filter is nullptr");
			return;
		}

		Engine->SetFogColor(FogColor);
		Engine->SetFogRange(FogRange);
		Engine->SetFogStart(FogStart);
	}
	else
	{
		Engine->SetFogRange(0);
		Engine->SetFogStart(INFINITE);
		Engine->SetFogColor({ 0, 0, 0, 1 });
	}
}