
#include "ShaderReader.h"

#include "GraphicsPipeline/GraphicsPipeline.h"

#include <ranges>

unordered_map<string, vector<SPipelineStage>> OShaderReader::LoadShaders()
{
	unordered_map<string, vector<SPipelineStage>> result;
	vector<SPipelineStage> currentPipeline;
	for (auto& shader : PTree.get_child("Shaders") | std::views::values)
	{
		SPipelineStage info;
		info.ShaderPath = UTF8ToWString(shader.get<string>("Path"));
		info.ShaderName = shader.get<string>("Name");
		for (auto& val : GetRootChild("Pipeline") | std::views::values)
		{
			SShaderDefinition def;
			def.TypeFromString(val.get<string>("Type"));
			def.ShaderEntry = val.get<string>("EntryPoint");
			def.TargetProfile = InferTargetProfile(def.ShaderType);
			info.ShaderDefinition = def;
			info.Defines = FindDefines(val);
			currentPipeline.push_back(info);
		}
		result[info.ShaderName] = currentPipeline;
	}
	return result;
}
vector<D3D_SHADER_MACRO> OShaderReader::FindDefines(const boost::property_tree::ptree& Tree)
{
	if (const auto Optional = Tree.get_child_optional("Defines"))
	{
		vector<D3D_SHADER_MACRO> result;
		for (auto& val : Optional.get() | std::views::values)
		{
			D3D_SHADER_MACRO macro;
			macro.Name = val.get<string>("Name").c_str();
			macro.Definition = val.get<string>("Value").c_str();
			result.push_back(macro);
		}
		return result;
	}
	return {};
}

string OShaderReader::InferTargetProfile(const EShaderLevel& ShaderType)
{
	switch (ShaderType)
	{
	case EShaderLevel::VertexShader:
		return "vs_5.1";
	case EShaderLevel::PixelShader:
		return "ps_5.1";
	case EShaderLevel::GeometryShader:
		return "gs_5.1";
	case EShaderLevel::HullShader:
		return "hs_5.1";
	case EShaderLevel::DomainShader:
		return "ds_5.1";
	case EShaderLevel::ComputeShader:
		return "cs_5.1";
	}
	return "";
}
