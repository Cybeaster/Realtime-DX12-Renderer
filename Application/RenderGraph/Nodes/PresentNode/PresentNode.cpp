
#include "PresentNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
#include "Window/Window.h"

ORenderTargetBase* OPresentNode::Execute(ORenderTargetBase* RenderTarget)
{
	auto window = OEngine::Get()->GetWindow();
	CommandQueue->ResourceBarrier(window, D3D12_RESOURCE_STATE_PRESENT);
	CommandQueue->ExecuteCommandList();
	THROW_IF_FAILED(window->GetSwapChain()->Present(0, 0));
	window->MoveToNextFrame();
	OEngine::Get()->CurrentFrameResources->Fence = CommandQueue->Signal();
	OEngine::Get()->FlushGPU();
	return RenderTarget;
}