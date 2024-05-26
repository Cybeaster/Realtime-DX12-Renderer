#pragma once
#include "DirectX/Resource.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"
#include "Engine/RenderTarget/SSAORenderTarget/Ssao.h"
#include "RenderGraph/Nodes/RenderNode.h"

class OSSAONode : public ORenderNode
{
	ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget) override;

public:
	weak_ptr<OSSAORenderTarget> GetSSAORT() const;
	void DrawNormals();
	void DrawSSAO();
	void BlurSSAO();
	void BlurSSAO(OSSAORenderTarget::ESubtargets OutputTarget, const SDescriptorPair* InputSRV, const SDescriptorPair* OutputRTV);
	void Update() override;
};
