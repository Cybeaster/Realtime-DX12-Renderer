#pragma once
#include "DXHelper.h"
#include "ShaderArray.h"
#include "Types.h"
struct SPSODescription
{
	string Name;
	SShaderArray ShaderPipeline;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc;
};
