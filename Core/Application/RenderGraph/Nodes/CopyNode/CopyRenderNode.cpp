
#include "CopyRenderNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Profiler.h"

ORenderTargetBase* OCopyRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	auto lock = OutTexture.lock();
	CommandQueue->SetRenderTarget(lock.get());
	CommandQueue->CopyResourceTo(lock.get(), RenderTarget);
	return lock.get();
}