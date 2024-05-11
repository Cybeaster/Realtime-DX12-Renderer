#pragma once
#include "DirectX/RenderItem/RenderItem.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"

struct SMaterial;
class OLightEmittingObject : public ORenderObjectBase
{
public:
	OLightEmittingObject() = default;
	OLightEmittingObject(const weak_ptr<SMeshGeometry>& Mesh, const weak_ptr<SMaterial>& Material);

private:
	weak_ptr<ORenderItem> RenderItem;
};
