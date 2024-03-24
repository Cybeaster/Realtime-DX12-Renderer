#include "ShadowMapNode.h"
#include "Engine/Engine.h"

ORenderTargetBase* OShadowMapNode::Execute(ORenderTargetBase* RenderTarget)
{
	for (auto map : OEngine::Get()->GetShadowMaps())
	{
		CommandQueue->SetRenderTarget(map);
		CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		CommandQueue->SetResource("cbPass",map->GetPassConstantAddresss(),PSO);
		OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::Opaque), SRenderLayer::Opaque);
		CommandQueue->ResourceBarrier(map, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	return RenderTarget;
}

void OShadowMapNode::SetupCommonResources()
{
	auto resource = OEngine::Get()->CurrentFrameResources;
	CommandQueue->SetPipelineState(PSO);
	CommandQueue->SetResource("cbPass", resource->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gMaterialData", resource->MaterialBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gTextureMaps", OEngine::Get()->NullTexSRV.GPUHandle, PSO);
	CommandQueue->SetResource("gCubeMap", OEngine::Get()->NullCubeSRV.GPUHandle, PSO);
	CommandQueue->SetResource("gDirectionalLights", resource->DirectionalLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gPointLights", resource->PointLightBuffer->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gSpotLights", resource->SpotLightBuffer->GetGPUAddress(), PSO);
}
