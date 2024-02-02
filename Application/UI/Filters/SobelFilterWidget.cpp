
#include "SobelFilterWidget.h"

#include "Filters/SobelFilter/SobelFilter.h"
void OSobelFilterWidget::Draw()
{
	if (ImGui::CollapsingHeader("Sobel filter"))
	{
		ImGui::Checkbox("Enable sobel", &bEnable);
	}
}
void OSobelFilterWidget::Update()
{
	IWidget::Update();
	Sobel->SetIsEnabled(bEnable);
}