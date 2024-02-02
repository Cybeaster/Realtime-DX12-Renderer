#pragma once
#include "Engine/Engine.h"
#include "GeometryEntity/GeometryEntityWidget.h"
#include "UI/Widget.h"

class OEngine;
class OGeometryManagerWidget : public IWidget
{
public:
	using TEntity = unique_ptr<OGeometryEntityWidget>;
	OGeometryManagerWidget(OEngine* _Engine, OEngine::TSceneGeometryMap* Map)
	    : Engine(_Engine), GeometryMap(Map) {}

	void Draw() override;
	void Update() override;

private:
	OEngine::TSceneGeometryMap* GeometryMap = nullptr;
	vector<TEntity> GeometryEntities;
	OEngine* Engine = nullptr;
};
