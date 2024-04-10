
#include "ShaderReader.h"

#include "Application.h"
#include "GraphicsPipeline/GraphicsPipeline.h"

#include <ranges>

unordered_map<string, vector<SPipelineStage>> OShaderReader::LoadShaders()
{
	unordered_map<string, vector<SPipelineStage>> result;
	for (auto& shader : PTree.get_child("Shaders") | std::views::values)
	{
		vector<SPipelineStage> currentPipeline;

		SPipelineStage info;
		info.ShaderPath = OApplication::Get()->GetResourcePath(UTF8ToWString(shader.get<string>("Path")));

		info.ShaderName = shader.get<string>("Name");
		for (auto& val : shader.get_child("Pipeline") | std::views::values)
		{
			SShaderDefinition def;
			def.TypeFromString(val.get<string>("Type"));
			def.ShaderEntry = UTF8ToWString(val.get<string>("EntryPoint"));
			def.TargetProfile = UTF8ToWString(val.get<string>("TargetProfile"));
			info.ShaderDefinition = def;
			info.Defines = FindDefines(val);
			currentPipeline.push_back(info);
		}
		result[info.ShaderName] = currentPipeline;
	}
	return result;
}
vector<SShaderMacro> OShaderReader::FindDefines(const boost::property_tree::ptree& Tree)
{
	if (const auto Optional = Tree.get_child_optional("Defines"))
	{
		vector<SShaderMacro> result;
		for (auto& val : Optional.get() | std::views::values)
		{
			SShaderMacro macro;
			macro.Name = val.get<string>("Name");
			macro.Definition = val.get<string>("Value");
			result.push_back(macro);
		}
		return result;
	}
	return {};
}
