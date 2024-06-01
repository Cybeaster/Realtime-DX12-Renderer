
#include "ReflectionNode.h"

#include "Engine/Engine.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "EngineHelper.h"
#include "Profiler.h"
#include "Window/Window.h"
void OReflectionNode::SetupCommonResources()
{
	auto pso = FindPSOInfo(PSO);
	auto resource = OEngine::Get()->CurrentFrameResource;

	auto cmdList = CommandQueue->GetCommandList();
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), GetSkyTextureSRV(), pso);
	CommandQueue->SetPipelineState(pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(MATERIAL_DATA), resource->MaterialBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(TEXTURE_MAPS), OEngine::Get()->TexturesStartAddress.GPUHandle, pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), GetSkyTextureSRV(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(DIRECTIONAL_LIGHTS), resource->DirectionalLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(POINT_LIGHTS), resource->PointLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SPOT_LIGHTS), resource->SpotLightBuffer->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SHADOW_MAPS), OEngine::Get()->GetRenderGroupStartAddress(ERenderGroup::ShadowTextures), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(SSAO_MAP), OEngine::Get()->GetSSAORT().lock()->GetAmbientMap0SRV().GPUHandle, pso);
}

ORenderTargetBase* OReflectionNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	auto pso = FindPSOInfo(PSO);

	auto cube = OEngine::Get()->GetCubeRenderTarget().lock();
	auto cmdList = CommandQueue->GetCommandList();
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), GetSkyTextureSRV(), pso);
	CommandQueue->SetPipelineState(pso);
	cube->SetViewport(CommandQueue->GetCommandList().Get());
	for (size_t i = 0; i < cube->GetNumRTVRequired(); i++)
	{
		CommandQueue->SetAndClearRenderTarget(cube.get(), i);
		CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), cube->GetPassConstantAddresss(i), pso);
		OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Opaque);
		OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);
	OEngine::Get()->SetWindowViewport(); // TODO remove this to other place
	CommandQueue->SetResource(STRINGIFY_MACRO(CB_PASS), OEngine::Get()->CurrentFrameResource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), cube->GetSRV().GPUHandle, pso);
	CommandQueue->SetRenderTarget(RenderTarget);
	OEngine::Get()->DrawRenderItems(pso, GetNodeInfo().RenderLayer);
	CommandQueue->SetResource(STRINGIFY_MACRO(CUBE_MAP), GetSkyTextureSRV(), pso);
	return RenderTarget;
}