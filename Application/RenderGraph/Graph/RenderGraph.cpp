
#include "RenderGraph.h"

#include "Application.h"
#include "RenderGraph/Nodes/PostProcessNode/PostProcessNode.h"
#include "RenderGraph/Nodes/PresentNode/PresentNode.h"
#include "RenderGraph/Nodes/ReflectionNode/ReflectionNode.h"
#include "RenderGraph/Nodes/UINode/UiRenderNode.h"

ORenderGraph::ORenderGraph()
{
	Reader = make_unique<ORenderGraphReader>(OApplication::Get()->GetConfigPath("RenderGraphConfigPath"));
}

void ORenderGraph::Initialize(OGraphicsPipelineManager* PipelineManager, OCommandQueue* OtherCommandQueue)
{
	this->PipelineManager = PipelineManager;
	auto graph = Reader->LoadRenderGraph();
	for (auto& node : graph)
	{
		auto newNode = ResolveNodeType(node.Name);
		if (Head == nullptr)
		{
			Head = newNode.get();
		}
		newNode->Initialize(node, OtherCommandQueue, this, PipelineManager->FindPSO(node.PSOType));
		Graph[node.Name] = newNode.get();
		Nodes.push_back(move(newNode));
	}
}

void ORenderGraph::Execute()
{
	auto currentNode = Head;
	ORenderTargetBase* texture = OEngine::Get()->GetOffscreenRT();
	while (currentNode != nullptr)
	{
		LOG(Render, Log, "Executing node: {}", TEXT(currentNode->GetNodeInfo().Name));
		texture = currentNode->Execute(texture);
		auto nextNode = Graph[currentNode->GetNextNode()];
		currentNode = nextNode;
	}
}
void ORenderGraph::SetPSO(const string& Type) const
{
	CommandQueue->SetPipelineState(PipelineManager->FindPSO(Type));
}

SPSODescriptionBase* ORenderGraph::FindPSOInfo(const string& Name) const
{
	return PipelineManager->FindPSO(Name);
}

//TODO Resolve automatically the type of the node
unique_ptr<ORenderNode> ORenderGraph::ResolveNodeType(const string& Type)
{
	if (Type == "OpaqueDynamicReflections")
	{
		return make_unique<OReflectionNode>();
	}
	if (Type == "Opaque")
	{
		return make_unique<ORenderNode>();
	}
	else if (Type == "Transparent")
	{
		return make_unique<ORenderNode>();
	}
	else if (Type == "PostProcess")
	{
		return make_unique<OPostProcessNode>();
	}
	else if (Type == "UI")
	{
		return make_unique<OUIRenderNode>();
	}
	else if (Type == "Present")
	{
		return make_unique<OPresentNode>();
	}
	LOG(Render, Error, "Node type not found: {}", TEXT(Type));
	return nullptr;
}
/*
*   {
	  "Name": "OpaqueDynamicReflections",
	  "PSO": "Opaque",
	  "NextNode": "Opaque",
	  "RenderLayer": "OpaqueDynamicReflections"
	},
	{
	  "Name": "Opaque",
	  "PSO": "Opaque",
	  "NextNode": "Transparent",
	  "RenderLayer": "Opaque"
	},
	{
	  "Name": "Transparent",
	  "PSO": "Transparent",
	  "NextNode": "PostProcess",
	  "RenderLayer": "Transparent"
	},
	{
	  "Name": "PostProcess",
	  "PSO": "PostProcess",
	  "NextNode": "UI",
	  "RenderLayer": "PostProcess"
	},
	{
	  "Name": "UI",
	  "PSO": "UI",
	  "RenderLayer": "UI",
	  "NextNode": "Resolve"
	},
	{
	  "Name": "Resolve",
	  "PSO": "Resolve",
	  "NextNode": "Present"
	},
	{
	  "Name": "Present",
	  "PSO": "Present",
	  "NextNode": ""
	}
 */