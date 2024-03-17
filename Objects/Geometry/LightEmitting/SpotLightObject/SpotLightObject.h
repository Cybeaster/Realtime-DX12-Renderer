#pragma once
#include "Engine/RenderObject/RenderObject.h"
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



