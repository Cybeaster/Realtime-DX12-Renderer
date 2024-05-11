#pragma once
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
	OGeometryEntityWidget(const weak_ptr<ORenderItem>& InRItem, OEngine* InEngine, OGeometryManagerWidget* InOwningWidget)
	    : RenderItem(InRItem), Engine(InEngine), Manager(InOwningWidget) {}

	void Draw() override;
	void InitWidget() override;
	SMeshGeometry* GetGeometry() const
	{
		if (!RenderItem.expired())
			return RenderItem.lock()->Geometry.lock().get();
		else
			return nullptr;
	}
	weak_ptr<ORenderItem> GetRenderItem() const { return RenderItem; }
	void RebuildRequest() const;

private:
	void DrawSubmeshParameters();
	void DrawInstanceParameters();
	float SliderWidth = 350;
	OGeometryManagerWidget* Manager = nullptr;
	weak_ptr<ORenderItem> RenderItem;
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
