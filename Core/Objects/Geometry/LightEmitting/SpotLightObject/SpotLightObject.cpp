
#include "SpotLightObject.h"

#include "Engine/Engine.h"

OSpotLightObject::OSpotLightObject(const HLSL::SpotLight& Data, SMeshGeometry* Mesh, SMaterial* Mat)
    : OLightEmittingObject(Mesh, Mat), LightData(Data)
{
}
