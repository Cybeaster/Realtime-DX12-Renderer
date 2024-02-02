#pragma once
#include "UI/Widget.h"

class OSobelFilter;

class OSobelFilterWidget : public IWidget
{
public:
	OSobelFilterWidget(OSobelFilter* Other)
	    : Sobel(Other) {}

	void Draw() override;
	void Update() override;

private:
	OSobelFilter* Sobel = nullptr;
	bool bEnable = false;
};
