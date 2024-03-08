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
	void Initialize(OGraphicsPipelineManager* PipelineManager, OCommandQueue* OtherCommandQueue);
	void Execute();
	void SetPSO(const string& Type) const;
	SPSODescriptionBase* FindPSOInfo(const string& Name) const;
	static unique_ptr<ORenderNode> ResolveNodeType(const string& Type);

private:
	unique_ptr<ORenderGraphReader> Reader;
	vector<unique_ptr<ORenderNode>> Nodes;
	ODependencyInfo Graph;
	ORenderNode* Head;
	OCommandQueue* CommandQueue;
	OGraphicsPipelineManager* PipelineManager;
};
