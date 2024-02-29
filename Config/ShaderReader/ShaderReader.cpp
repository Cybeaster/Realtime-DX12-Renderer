
#include "ShaderReader.h"

#include "GraphicsPipeline/GraphicsPipeline.h"

#include <ranges>

vector<SShaderInfo> OShaderReader::LoadShaders()
{
	vector<SShaderInfo> result;
	for (auto& shader : PTree.get_child("Shaders") | std::views::values)
	{
		SShaderInfo info;
		info.ShaderPath = shader.get<wstring>("Path");
		info.ShaderName = shader.get<string>("Name");
		for (auto& val : GetRootChild("Pipeline") | std::views::values)
		{
			SShaderDefinition def;
			def.TypeFromString(val.get<string>("Type"));
			def.ShaderEntry = val.get<string>("EntryPoint");
			info.Definitions.push_back(def);
		}
	}
	return result;
}