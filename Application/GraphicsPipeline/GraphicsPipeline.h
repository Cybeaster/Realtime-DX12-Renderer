#pragma once
#include "DXHelper.h"
#include "Engine/Shader/Shader.h"
#include "Types.h"


class OGraphicsPipeline
{
public:
private:
	ComPtr<ID3D12PipelineState> PipelineState;
	SPipelineInfo PipelineInfo;
	vector<OShader*> Shaders;
};
