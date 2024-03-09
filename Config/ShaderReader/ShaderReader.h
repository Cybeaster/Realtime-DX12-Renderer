#pragma once
#include "ConfigReader.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "Types.h"
class OShaderReader : public OConfigReader
{
public:
	OShaderReader(const string& ConfigPath)
	    : OConfigReader(ConfigPath)
	{
	}
	unordered_map<string, vector<SPipelineStage>> LoadShaders();

private:
	vector<D3D_SHADER_MACRO> FindDefines(const boost::property_tree::ptree& Tree);
};
