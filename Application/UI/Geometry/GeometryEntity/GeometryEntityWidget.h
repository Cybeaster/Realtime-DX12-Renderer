#pragma once
#include "GeometryTransform.h"
#include "RenderItem.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

struct SRenderItem;
class OGeometryManagerWidget;
class OEngine;
struct SMeshGeometry;
class OGeometryEntityWidget : public OHierarchicalWidgetBase
{
public:
	OGeometryEntityWidget(SRenderItem* _RItem, OEngine* _Engine, OGeometryManagerWidget* _OwningWidget)
	    : RenderItem(_RItem), Engine(_Engine), Manager(_OwningWidget) {}

	void Draw() override;
	void Update() override;
	void Init() override;
	SMeshGeometry* GetGeometry() const
	{
		return RenderItem->Geometry;
	}
	void RebuildRequest();

private:
	float SliderWidth = 350;
	OGeometryManagerWidget* Manager = nullptr;
	SRenderItem* RenderItem = nullptr;
	OEngine* Engine = nullptr;

	string SelectedSubmesh = "";
	string SelectedInstance = "";

	SInstanceData* SelectedInstanceData = nullptr;


	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;



	OGeometryTransformWidget* TransformWidget = nullptr;
};
