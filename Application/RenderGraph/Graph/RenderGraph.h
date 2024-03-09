#pragma once
#include "RenderGraph/Nodes/RenderNode.h"
#include "RenderGraphReader/RenderGraphReader.h"
#include "Types.h"

struct SPSODescriptionBase;
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
	ORenderNode* Head = nullptr;
	OCommandQueue* CommandQueue;
	OGraphicsPipelineManager* PipelineManager;
};
