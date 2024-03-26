
#include "RenderGraph.h"

#include "Application.h"
#include "RenderGraph/Nodes/DefaultNode/DefaultRenderNode.h"
#include "RenderGraph/Nodes/PostProcessNode/PostProcessNode.h"
#include "RenderGraph/Nodes/PresentNode/PresentNode.h"
#include "RenderGraph/Nodes/ReflectionNode/ReflectionNode.h"
#include "RenderGraph/Nodes/ShadowNode/ShadowMapNode.h"
#include "RenderGraph/Nodes/UINode/UiRenderNode.h"

ORenderGraph::ORenderGraph()
{
	Reader = make_unique<ORenderGraphReader>(OApplication::Get()->GetConfigPath("RenderGraphConfigPath"));
}

void ORenderGraph::Initialize(OGraphicsPipelineManager* PipelineManager, OCommandQueue* OtherCommandQueue)
{
	this->PipelineManager = PipelineManager;
	CommandQueue = OtherCommandQueue;
	string head;
	auto graph = Reader->LoadRenderGraph(head);
	for (auto& node : graph)
	{
		auto newNode = ResolveNodeType(node.Name);
		if (head == node.Name)
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
	auto engine = OEngine::Get();
	if (engine->GetDescriptorHeap())
	{
		auto currentNode = Head;
		engine->GetWindow()->SetViewport(CommandQueue->GetCommandList().Get());
		ORenderTargetBase* texture = OEngine::Get()->GetOffscreenRT();
		engine->SetDescriptorHeap(EResourceHeapType::Default);
		CommandQueue->SetRenderTarget(texture);
		while (currentNode != nullptr)
		{
			if (!currentNode->GetNodeInfo().bEnable)
			{
				currentNode = Graph[currentNode->GetNextNode()];
				continue;
			}
			LOG(Render, Log, "Executing node: {}", TEXT(currentNode->GetNodeInfo().Name));
			currentNode->SetupCommonResources();
			texture = currentNode->Execute(texture);
			currentNode = Graph[currentNode->GetNextNode()];
		}
	}
	else
	{
		LOG(Render, Error, "SRVHeap is not initialized!");
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

// TODO Resolve automatically the type of the node
unique_ptr<ORenderNode> ORenderGraph::ResolveNodeType(const string& Type)
{
	if (Type == "OpaqueDynamicReflections")
	{
		return make_unique<OReflectionNode>();
	}
	if (Type == "Opaque")
	{
		return make_unique<ODefaultRenderNode>();
	}
	else if (Type == "Transparent")
	{
		return make_unique<ODefaultRenderNode>();
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
	else if (Type == "Shadow")
	{
		return make_unique<OShadowMapNode>();
	}
	LOG(Render, Warning, "Node type not found: {}", TEXT(Type));
	return make_unique<ODefaultRenderNode>();
}
