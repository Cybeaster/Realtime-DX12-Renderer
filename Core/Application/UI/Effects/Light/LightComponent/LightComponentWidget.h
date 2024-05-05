
#pragma once
#include "UI/Widget.h"

class OLightComponent;
class OLightComponentWidget : public IWidget
{
public:
	void Draw() override;
	void SetComponent(OLightComponent* InComponent);
private:
	OLightComponent* LightComponent = nullptr;
};



