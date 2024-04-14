
#include "DirectX/RenderItem/RenderItem.h"
#include "Engine/Engine.h"
#include "OLightEmittingObject.h"
OLightEmittingObject::OLightEmittingObject(SMeshGeometry* Mesh, SMaterial* Material)
{
	SRenderItemParams params;
	params.MaterialParams.Material = Material;
	RenderItem = OEngine::Get()->BuildRenderItemFromMesh(Mesh, params);
}
