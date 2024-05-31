#pragma once
#include "DirectX/Light/Light.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

class OEngine;
class OLightWidget : public IWidget
{
	struct SWidgetLight
	{
		int32_t Index = -1;
		string Name = "";
	};

public:
	OLightWidget(OEngine* Arg)
	    : Engine(Arg){};

	void Draw() override;
	void InitWidget() override;
	void Update() override;

private:
	DirectX::XMFLOAT4 AmbientColor = { 0.1f, 0.1f, 0.1f, 0.32f };
	map<int32_t, SWidgetLight> Lights;
	int32_t SelectedLightIdx = -1;
	OEngine* Engine = nullptr;
};
