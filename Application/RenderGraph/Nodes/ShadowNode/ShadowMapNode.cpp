#include "ShadowMapNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"

ORenderTargetBase* OShadowMapNode::Execute(ORenderTargetBase* RenderTarget)
{
	for (auto map : OEngine::Get()->GetShadowMaps())
	{
		CommandQueue->SetRenderTarget(map);
		CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		CommandQueue->SetResource("cbPass", map->GetPassConstantAddresss(), PSO);
		OEngine::Get()->DrawRenderItems(PSO, SRenderLayer::Opaque);
		CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	OEngine::Get()->SetWindowViewport(); // TODO remove this to other place
	CommandQueue->SetRenderTarget(RenderTarget);
	return RenderTarget;
}

void OShadowMapNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResource;
	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart(), PSO);
}
