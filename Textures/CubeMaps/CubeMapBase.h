#pragma once
#include "Types.h"

struct STexture;
class OCubeMapBase
{
	OCubeMapBase(const wstring& TexturePath);
	void Init();

private:
	STexture* SkyTexture;
};
