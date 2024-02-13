#pragma once
#include "GeometryTransform.h"
#include "RenderItem.h"
#include "UI/Material/MaterialPicker.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

struct SMeshGeometry;
struct SRenderItem;
class OGeometryManagerWidget;
class OEngine;

class OGeometryEntityWidget : public OHierarchicalWidgetBase
{
public:
	OGeometryEntityWidget(SRenderItem* _RItem, OEngine* _Engine, OGeometryManagerWidget* _OwningWidget)
	    : RenderItem(_RItem), Engine(_Engine), Manager(_OwningWidget) {}

	void Draw() override;
	void Init() override;
	SMeshGeometry* GetGeometry() const
	{
		return RenderItem->Geometry;
	}
	void RebuildRequest() const;

private:
	void DrawSubmeshParameters();
	void DrawInstanceParameters();

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
	OMaterialPickerWidget* MaterialPickerWidget = nullptr;
};
