
#include "SpotLightObject.h"

#include "Engine/Engine.h"

OSpotLightObject::OSpotLightObject(const SSpotLight& Data, SMeshGeometry* Mesh, SMaterial* Mat) : OLightEmittingObject(Mesh, Mat), LightData(Data)
{

}
