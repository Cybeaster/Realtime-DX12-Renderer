#pragma once
#include "DirectX/ShaderTypes.h"
#include "Engine/Shader/Shader.h"

class OGraphicsPipeline
{
private:
	ComPtr<ID3D12PipelineState> PipelineState;
	SShaderPipelineDesc PipelineInfo = {};
	vector<OShader*> Shaders;
};
