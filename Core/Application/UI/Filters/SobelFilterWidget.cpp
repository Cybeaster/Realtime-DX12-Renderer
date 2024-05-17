
#include "SobelFilterWidget.h"

#include "Engine/RenderTarget/Filters/SobelFilter/SobelFilter.h"
void OSobelFilterWidget::Draw()
{
	bEnable = ImGui::TreeNode("Sobel Filter");
	if (bEnable)
	{
		ImGui::Checkbox("Pure Sobel", &PureSobel);
		ImGui::TreePop();
	}
}
void OSobelFilterWidget::Update()
{
	IWidget::Update();
	Sobel->SetIsEnabled(bEnable, PureSobel);
}
