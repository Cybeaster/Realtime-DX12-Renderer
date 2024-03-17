#pragma once
#include "Engine/Engine.h"
#include "GeometryEntity/GeometryEntityWidget.h"
#include "UI/Effects/Light/LightComponent/LightComponentWidget.h"
#include "UI/Widget.h"

class OPickedRenderItemWidget;
class OEngine;
class OGeometryManagerWidget : public OHierarchicalWidgetBase
{
public:
	using TEntity = unique_ptr<OGeometryEntityWidget>;
	OGeometryManagerWidget(OEngine* _Engine, OEngine::TRenderLayer* Map)
	    : Engine(_Engine), RenderLayers(Map) {}

	void Draw() override;
	void InitWidget() override;
	void RebuildRequest() const;
	void DrawComponentWidgets();

private:
	OPickedRenderItemWidget* PickedRenderItemWidget = nullptr;
	OEngine::TRenderLayer* RenderLayers = nullptr;
	OLightComponentWidget* LightComponentWidget = nullptr;
	OEngine* Engine = nullptr;
	string SelectedRenderItem = "";
	string SelectedComponentName = "";
	OComponentBase* SelectedComponent = nullptr; // todo remove
	unordered_map<string,ORenderItem*> StringToGeo;
};
