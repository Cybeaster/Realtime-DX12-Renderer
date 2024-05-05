#pragma once
#include "UI/Widget.h"

class OGaussianBlurFilter;
class OGaussianBlurWidget : public IWidget
{
public:
	OGaussianBlurWidget(OGaussianBlurFilter* Arg)
	    : Filter(Arg){};
	void Draw() override;
	void Update() override;

private:
	bool bEnabled = false;
	float Sigma = 1.0f;
	int32_t BlurCount = 1;
	OGaussianBlurFilter* Filter = nullptr;
};