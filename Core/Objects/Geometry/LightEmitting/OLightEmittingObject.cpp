
#include "OLightEmittingObject.h"

#include "DirectX/RenderItem/RenderItem.h"
#include "Engine/Engine.h"
OLightEmittingObject::OLightEmittingObject(const weak_ptr<SMeshGeometry>& Mesh, const weak_ptr<SMaterial>& Material)
{
	SRenderItemParams params;
	params.MaterialParams.Material = Material;
	RenderItem = OEngine::Get()->BuildRenderItemFromMesh(Mesh, params);
}
