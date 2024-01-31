
#include "BilateralFilterWidget.h"

#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Logger.h"
void OBilateralBlurFilterWidget::Draw()
{
	if (ImGui::CollapsingHeader("Bilateral Blur"))
	{
		ImGui::Checkbox("Is Bilateral filter Enabled", &bEnabled);
		ImGui::SliderFloat("Bilateral Spatial Sigma", &SpatialSigma, 0.0f, 50.0f);
		ImGui::SliderFloat("Bilateral Intensity Sigma", &IntensitySigma, 0.0f, 25.0f);
		ImGui::SliderInt("Bilateral Blur Count", &BlurCount, 0, 15);
	}
}

void OBilateralBlurFilterWidget::Update()
{
	if (bEnabled)
	{
		if (Filter == nullptr)
		{
			LOG(Widget, Error, "Filter is nullptr");
			return;
		}

		Filter->SetBlurCount(BlurCount);
		Filter->SetIntensitySigma(IntensitySigma);
		Filter->SetSpatialSigma(SpatialSigma);
	}
	else
	{
		Filter->SetBlurCount(0);
	}
}