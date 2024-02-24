#pragma once
#include "../ConfigReader.h"

struct STexture;
class OTexturesParser : public OConfigReader
{
public:
	OTexturesParser(const string& ConfigPath)
	    : OConfigReader(ConfigPath)
	{
		LoadConfig(FileName);
	}

	void AddTexture(STexture* Texture);
	void AddTextures(vector<STexture*> Textures);
	vector<STexture*> LoadTextures();
};
