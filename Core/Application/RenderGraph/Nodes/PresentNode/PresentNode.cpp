
#include "PresentNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Engine/Engine.h"
#include "Profiler.h"
#include "Window/Window.h"

ORenderTargetBase* OPresentNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	auto window = OEngine::Get()->GetWindow().lock();
	CommandQueue->ResourceBarrier(window.get(), D3D12_RESOURCE_STATE_PRESENT);
	CommandQueue->ExecuteCommandList();
	THROW_IF_FAILED(window->GetSwapChain()->Present(0, 0));
	THROW_IF_FAILED(OEngine::Get()->GetDevice().lock()->GetDevice()->GetDeviceRemovedReason());
	window->MoveToNextFrame();
	OEngine::Get()->CurrentFrameResource->Fence = CommandQueue->Signal();
	OEngine::Get()->FlushGPU();
	return RenderTarget;
}