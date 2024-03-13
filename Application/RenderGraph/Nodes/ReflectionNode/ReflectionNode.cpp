
#include "ReflectionNode.h"

#include "Engine/Engine.h"
#include "EngineHelper.h"
#include "Window/Window.h"
ORenderTargetBase* OReflectionNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto cube = OEngine::Get()->GetCubeRenderTarget();
	auto cmdList = CommandQueue->GetCommandList();
	UINT passCBByteSize = Utils::CalcBufferByteSize(sizeof(SPassConstants));
	auto resource = OEngine::Get()->CurrentFrameResources->PassCB->GetGPUAddress();
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(),PSO);

	cube->SetViewport(CommandQueue->GetCommandList().Get());
	for (size_t i = 0; i < cube->GetNumRTVRequired(); i++)
	{
		CommandQueue->SetRenderTarget(cube, i);
	  	CommandQueue->SetResource("cbPass", resource + i * passCBByteSize, PSO);
		OEngine::Get()->DrawRenderItems(PSO->RootSignature.get(), SRenderLayer::Opaque);
		//OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::WavesRender)->RootSignature.get(), SRenderLayer::Waves);
	    OEngine::Get()->DrawRenderItems(FindPSOInfo(SPSOType::Sky)->RootSignature.get(), SRenderLayer::Sky);
	}
	Utils::ResourceBarrier(cmdList.Get(), cube->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ);

	OEngine::Get()->SetWindowViewport(); //TODO remove this to other place

	CommandQueue->SetResource("cbPass", OEngine::Get()->CurrentFrameResources->PassCB->GetGPUAddress(), PSO);
	CommandQueue->SetResource("gCubeMap", cube->GetSRVHandle().GPUHandle, PSO);

	CommandQueue->SetRenderTarget(RenderTarget);

	OEngine::Get()->DrawRenderItems(PSO->RootSignature.get(), GetNodeInfo().RenderLayer);
	CommandQueue->SetResource("gCubeMap", GetSkyTextureSRV(),PSO);
	return RenderTarget;
}