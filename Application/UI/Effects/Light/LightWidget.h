#pragma once
#include "Light/Light.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

class OEngine;
class OLightWidget : public IWidget
{
	struct SWidgetLight
	{
		SLight Light;
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
	static inline char LightBuffer[256] = {};
	DirectX::XMFLOAT3 AmbientColor = { 0.0f, 0.0f, 0.0f };
	map<int32_t, SWidgetLight> Lights;
	int32_t SelectedLightIdx = -1;
	OEngine* Engine = nullptr;
};
