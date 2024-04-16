#include "ShadowMapNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"
#include "Profiler.h"

ORenderTargetBase* OShadowMapNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();

	for (auto map : OEngine::Get()->GetShadowMaps())
	{
		if (map->ConsumeUpdate())
		{
			CommandQueue->SetRenderTarget(map);
			CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			CommandQueue->SetResource("cbPass", map->GetPassConstantAddresss(), PSO);
			OEngine::Get()->DrawRenderItems(PSO, SRenderLayers::Opaque, true);
			CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
	}
	OEngine::Get()->SetWindowViewport(); // TODO remove this to other place
	CommandQueue->SetRenderTarget(RenderTarget);
	return RenderTarget;
}

void OShadowMapNode::SetupCommonResources()
{
	PROFILE_SCOPE();
	auto resource = OEngine::Get()->CurrentFrameResource;
	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
}
