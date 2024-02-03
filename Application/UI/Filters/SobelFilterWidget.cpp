
#include "SobelFilterWidget.h"

#include "Filters/SobelFilter/SobelFilter.h"
void OSobelFilterWidget::Draw()
{
	ImGui::Checkbox("Enable sobel Filter", &bEnable);
}
void OSobelFilterWidget::Update()
{
	IWidget::Update();
	Sobel->SetIsEnabled(bEnable);
}