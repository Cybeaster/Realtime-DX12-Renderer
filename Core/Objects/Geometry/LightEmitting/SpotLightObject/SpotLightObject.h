#pragma once
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Geometry/LightEmitting/OLightEmittingObject.h"

struct SMaterial;
class OSpotLightObject : public OLightEmittingObject
{
public:
	OSpotLightObject(const HLSL::SpotLight& Data, const weak_ptr<SMeshGeometry>& Mesh, const weak_ptr<SMaterial>& Mat);

public:
private:
	HLSL::SpotLight LightData;
};
