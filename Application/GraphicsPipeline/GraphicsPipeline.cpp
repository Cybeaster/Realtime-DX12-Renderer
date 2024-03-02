#include "GraphicsPipeline.h"

#include "../../Utils/EngineHelper.h"

D3D12_SHADER_BYTECODE SPipelineStage::GetShaderByteCode() const
{
	if (Shader)
	{
		return Shader->GetShaderByteCode();
	}
	return {};
}
