#pragma once
#include "Engine/RenderTarget/Filters/BilateralBlur/BilateralBlurFilter.h"
#include "UI/Widget.h"

class OBilateralBlurFilterWidget : public IWidget
{
public:
	OBilateralBlurFilterWidget(OBilateralBlurFilter* Filter)
	    : Filter(Filter){};

	void Draw() override;
	void Update() override;
	bool IsEnabled() override
	{
		return bEnabled;
	}

private:
	float SpatialSigma = 1.0f;
	float IntensitySigma = 1.0f;
	int32_t BlurCount = 1;

	bool bEnabled = false;

	OBilateralBlurFilter* Filter = nullptr;
};