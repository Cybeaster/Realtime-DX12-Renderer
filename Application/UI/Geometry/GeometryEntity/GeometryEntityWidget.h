#pragma once
#include "UI/Widget.h"

class OEngine;
struct SMeshGeometry;
class OGeometryEntityWidget : public IWidget
{
	OGeometryEntityWidget(SMeshGeometry* _Geometry, OEngine* _Engine)
	    : Geometry(_Geometry), Engine(_Engine) {}

	void Draw() override;
	void Update() override;

private:
	SMeshGeometry* Geometry = nullptr;
	OEngine* Engine = nullptr;
	string SelectedSubmesh = "";
	bool bRebuildRequested = false;
};
