#pragma once
#include "DXHelper.h"
#include "RenderNodeInfo.h"
#include "ShaderTypes.h"
#include "Types.h"
class ORenderNode
{
public:
	void Initialize(SShadersPipeline* Other, const SNodeInfo& OtherNodeInfo);
	string Execute();
	void SetupCommonResources();
	const SNodeInfo& GetNodeInfo() const { return NodeInfo; }

private:
	SShadersPipeline* PipelineInfo = nullptr;
	SNodeInfo NodeInfo;
};
