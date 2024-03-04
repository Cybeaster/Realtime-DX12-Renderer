#include "RenderNode.h"

#include "Engine/Engine.h"

void ORenderNode::Initialize(SShadersPipeline* Other, const SNodeInfo& OtherNodeInfo)
{
	PipelineInfo = Other;
	NodeInfo = OtherNodeInfo;
}

string ORenderNode::Execute()
{
	OEngine::Get()->DrawRenderItems(PipelineInfo->PipelineInfo.PipelineState.Get(), NodeInfo.RenderLayer);
	return NodeInfo.NextNode;
}

void ORenderNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;
	auto cmdList = OEngine::Get()->GetCommandQueue()->GetCommandList();
	auto passCB = resource->PassCB->GetResource();
	auto passCBByteSize = Utils::CalcBufferByteSize(sizeof(SPassConstants));
	auto param = PipelineInfo->PipelineInfo.RootParamIndexMap[L"PassCB"];
}