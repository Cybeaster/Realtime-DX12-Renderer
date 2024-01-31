
#include "FilterWidget.h"

#include "Filters/BilateralBlur/BilateralBlurFilter.h"
#include "Logger.h"
void OBilateralBlurFilterWidget::Draw()
{
	ImGui::Begin("Bilateral Blur Filter");
	ImGui::Checkbox("Is Enabled", &bEnabled);
	ImGui::SliderFloat("Spatial Sigma", &SpatialSigma, 0.0f, 50.0f);
	ImGui::SliderFloat("Intensity Sigma", &IntensitySigma, 0.0f, 25.0f);
	ImGui::SliderInt("Blur Count", &BlurCount, 0, 15);
	ImGui::End();
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