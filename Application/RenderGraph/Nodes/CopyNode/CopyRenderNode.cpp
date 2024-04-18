
#include "CopyRenderNode.h"

#include "CommandQueue/CommandQueue.h"
#include "Profiler.h"

ORenderTargetBase* OCopyRenderNode::Execute(ORenderTargetBase* RenderTarget)
{
	PROFILE_SCOPE();
	CommandQueue->SetRenderTarget(OutTexture);
	CommandQueue->CopyResourceTo(OutTexture, RenderTarget);
	return RenderTarget;
}