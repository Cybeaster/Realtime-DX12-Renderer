#include "CubeMapBase.h"

#include "../../Utils/EngineHelper.h"

OCubeMapBase::OCubeMapBase(const wstring& TexturePath)
{
	SkyTexture = FindOrCreateTexture(TexturePath);
	Init();
}

void OCubeMapBase::Init()
{
}