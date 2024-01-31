#pragma once
#include "Logger.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

class OEngine;
class OFogWidget : public IWidget
{
public:
	OFogWidget(OEngine* Arg)
	    : Engine(Arg){};

	void Draw() override;
	void Update() override;

private:
	bool bEnabled = false;
	OEngine* Engine = nullptr;
	float FogStart = 0.0f;
	float FogRange = 200.f;
	DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
};
