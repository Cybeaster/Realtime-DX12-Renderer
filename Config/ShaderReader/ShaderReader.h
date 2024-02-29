#pragma once
#include "../ConfigReader.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "Types.h"
class OShaderReader : public OConfigReader
{
public:
	OShaderReader(const string& ConfigPath)
	    : OConfigReader(ConfigPath)
	{
	}
	vector<SShaderInfo> LoadShaders();
};
