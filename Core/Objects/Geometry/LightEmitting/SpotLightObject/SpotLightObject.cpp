
#include "SpotLightObject.h"

#include "Engine/Engine.h"

OSpotLightObject::OSpotLightObject(const HLSL::SpotLight& Data, const weak_ptr<SMeshGeometry>& Mesh, const weak_ptr<SMaterial>& Mat)
    : OLightEmittingObject(Mesh, Mat), LightData(Data)
{
}
