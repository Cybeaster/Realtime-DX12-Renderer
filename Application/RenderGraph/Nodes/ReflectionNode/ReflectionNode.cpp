
#include "ReflectionNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"
#include "Profiler.h"
#include "Window/Window.h"
ORenderTargetBase* OReflectionNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	auto pso = FindPSOInfo(PSO);

	auto cube = OEngine::Get()->GetCubeRenderTarget();
	auto cmdList = CommandQueue->GetCommandList();
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), pso);

	cube->SetViewport(CommandQueue->GetCommandList().Get());
	for (size_t i = 0; i < cube->GetNumRTVRequired(); i++)
	{
		CommandQueue->SetPipelineState(pso);
		CommandQueue->SetRenderTarget(cube, i);
		CommandQueue->SetResource("cbPass", cube->GetPassConstantAddresss(i), pso);
		OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Opaque);
		OEngine::Get()->DrawRenderItems(pso, SRenderLayers::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	OEngine::Get()->SetWindowViewport(); // TODO remove this to other place

	CommandQueue->SetResource("cbPass", OEngine::Get()->CurrentFrameResource->PassCB->GetGPUAddress(), pso);
	CommandQueue->SetResource("gCubeMap", cube->GetSRV().GPUHandle, pso);

	CommandQueue->SetRenderTarget(RenderTarget);

	OEngine::Get()->DrawRenderItems(pso, GetNodeInfo().RenderLayer);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(), pso);
	return RenderTarget;
}