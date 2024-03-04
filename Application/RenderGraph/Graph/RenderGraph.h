#pragma once
#include "../../../Config/RenderGraphReader/RenderGraphReader.h"
#include "DXHelper.h"
#include "RenderGraph/Nodes/RenderNode.h"
#include "RenderNodeInfo.h"
#include "Types.h"

class OGraphicsPipelineManager;
class ORenderGraph
{
public:
	ORenderGraph();
	using ODependencyInfo = unordered_map<string, ORenderNode*>;
	void Initialize(OGraphicsPipelineManager* PipelineManager);
	void Execute();

private:
	unique_ptr<ORenderGraphReader> Reader;
	vector<unique_ptr<ORenderNode>> Nodes;
	ODependencyInfo Graph;
	ORenderNode* Head;
};
