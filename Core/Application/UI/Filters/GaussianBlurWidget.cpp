#include "GaussianBlurWidget.h"

#include "Engine/RenderTarget/Filters/Blur/BlurFilter.h"
void OGaussianBlurWidget::Draw()
{
	bEnabled = ImGui::TreeNode("Gaussian Blur");
	if (bEnabled)
	{
		ImGui::SliderFloat("Gaussian Sigma", &Sigma, 0.0f, 1.0f);
		ImGui::SliderInt("Gaussian Blur Count", &BlurCount, 1, 15);
		ImGui::TreePop();
	}
}

void OGaussianBlurWidget::Update()
{
	if (bEnabled)
	{
		Filter->SetParameters(BlurCount, Sigma);
	}
	else
	{
		Filter->SetParameters(1, 0);
	}
}