#pragma once
#include "DirectX/InstanceData.h"
#include "DirectX/RenderItem/RenderItem.h"
#include "GeometryTransform.h"
#include "UI/Material/MaterialPicker.h"
#include "UI/Widget.h"

#include <DirectXMath.h>

struct SMeshGeometry;
struct ORenderItem;
class OGeometryManagerWidget;
class OEngine;

class OGeometryEntityWidget : public OHierarchicalWidgetBase
{
public:
	OGeometryEntityWidget(ORenderItem* _RItem, OEngine* _Engine, OGeometryManagerWidget* _OwningWidget)
	    : RenderItem(_RItem), Engine(_Engine), Manager(_OwningWidget) {}

	void Draw() override;
	void InitWidget() override;
	SMeshGeometry* GetGeometry() const
	{
		if (RenderItem)
			return RenderItem->Geometry;
		else
			return nullptr;
	}
	void RebuildRequest() const;

private:
	void DrawSubmeshParameters();
	void DrawInstanceParameters();

	float SliderWidth = 350;
	OGeometryManagerWidget* Manager = nullptr;
	ORenderItem* RenderItem = nullptr;
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
