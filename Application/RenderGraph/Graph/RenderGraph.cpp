
#include "RenderGraph.h"

#include "Application.h"

ORenderGraph::ORenderGraph()
{
	Reader = make_unique<ORenderGraphReader>(OApplication::Get()->GetConfigPath("RenderGraphConfigPath"));
}

void ORenderGraph::Initialize(OGraphicsPipelineManager* PipelineManager)
{
	auto graph = Reader->LoadRenderGraph();
	for (auto& node : graph)
	{
		auto newNode = make_unique<ORenderNode>();
		if (Head == nullptr)
		{
			Head = newNode.get();
		}
		newNode->Initialize(PipelineManager->FindPipeline(node.PSOType), node);

		Graph[node.Name] = newNode.get();
		Nodes.push_back(move(newNode));
	}
}

void ORenderGraph::Execute()
{
	auto currentNode = Head;
	while (currentNode != nullptr)
	{
		LOG(Render, Log, "Executing node: {}", TEXT(currentNode->GetNodeInfo().Name));
		auto nextNode = Graph[currentNode->Execute()];
		currentNode = nextNode;
	}
}