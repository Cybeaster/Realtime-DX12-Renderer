#include "GraphicsPipeline.h"

#include "../../Utils/EngineHelper.h"

void OGraphicsPipeline::BuildShaders(const SShaderInfo& ShaderInfo)
{
	for (const auto& info : ShaderInfo.Definitions)
	{
		SPipelineInfo localInfo = {};
		auto shader = CompilerShader(info, ShaderInfo.ShaderPath, localInfo);
		if (PipelineInfo.RootParameters.empty())
		{
			PipelineInfo = std::move(localInfo);
		}

		Shaders.push_back(std::move(shader));
	}
}