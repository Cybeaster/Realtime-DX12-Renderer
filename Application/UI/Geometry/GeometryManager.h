#pragma once
#include "Engine/Engine.h"
#include "GeometryEntity/GeometryEntityWidget.h"
#include "UI/Widget.h"

class OEngine;
class OGeometryManagerWidget : public OHierarchicalWidgetBase
{
public:
	using TEntity = unique_ptr<OGeometryEntityWidget>;
	OGeometryManagerWidget(OEngine* _Engine, OEngine::TRenderLayer* Map)
	    : Engine(_Engine), RenderLayers(Map) {}

	void Draw() override;
	void Update() override;
	void Init() override;
	void RebuildRequest();

private:
	OEngine::TRenderLayer* RenderLayers = nullptr;
	OEngine* Engine = nullptr;
	string SelectedGeometry = "";
	bool bRebuildRequested = false;
};
