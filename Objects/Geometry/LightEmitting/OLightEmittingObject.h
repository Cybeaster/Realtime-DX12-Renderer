#pragma once
#include "DirectX/RenderItem/RenderItem.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"

struct SMaterial;
class OLightEmittingObject : public ORenderObjectBase
{
public:
	OLightEmittingObject() = default;
	OLightEmittingObject(SMeshGeometry* Mesh, SMaterial* Material);

private:
	ORenderItem* RenderItem = nullptr;
};
