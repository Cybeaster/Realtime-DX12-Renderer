#pragma once
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Geometry/LightEmitting/OLightEmittingObject.h"

struct SMaterial;
class OSpotLightObject : public OLightEmittingObject
{
public:
	OSpotLightObject(const SSpotLight& Data, SMeshGeometry* Mesh, SMaterial*);

public:
private:
	SSpotLight LightData;
};
