
#include "PresentNode.h"

#include "../../../../Utils/DirectXUtils.h"
#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"

ORenderTargetBase* OPresentNode::Execute(ORenderTargetBase* RenderTarget)
{
	Utils::ResourceBarrier(CommandQueue->GetCommandList().Get(), RenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandQueue->ExecuteCommandList();
	auto window = OEngine::Get()->GetWindow();
	THROW_IF_FAILED(window->GetSwapChain()->Present(0, 0));
	window->MoveToNextFrame();
	OEngine::Get()->CurrentFrameResources->Fence = CommandQueue->Signal();
	OEngine::Get()->FlushGPU();
	return RenderTarget;
}