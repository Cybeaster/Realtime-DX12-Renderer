#pragma once
#include "RenderItemComponentBase.h"

struct SSubmeshGeometry;
struct SMaterial;
struct SMeshGeometry;
class ORenderItemMeshComponent : public OComponentBase
{
public:
	SMaterial* DefaultMaterial = nullptr;
	SMeshGeometry* Geometry = nullptr;
	SSubmeshGeometry* ChosenSubmesh = nullptr;
};

