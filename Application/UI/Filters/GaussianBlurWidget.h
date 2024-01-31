#pragma once
#include "UI/Widget.h"

class OBlurFilter;
class OGaussianBlurWidget : public IWidget
{
public:
	OGaussianBlurWidget(OBlurFilter* Arg)
	    : Filter(Arg){};
	void Draw() override;
	void Update() override;

private:
	bool bEnabled = false;
	float Sigma = 1.0f;
	int32_t BlurCount = 1;
	OBlurFilter* Filter = nullptr;
};