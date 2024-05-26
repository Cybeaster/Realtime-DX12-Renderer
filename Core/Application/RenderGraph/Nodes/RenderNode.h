#pragma once
#include "DirectX/DXHelper.h"
#include "DirectX/RenderConstants.h"
#include "RenderNodeInfo.h"

struct SFrameResource;
struct SPSODescriptionBase;
class ORenderGraph;
class ORenderTargetBase;
class OCommandQueue;

class ORenderNode
{
public:
	ORenderNode() = default;
	ORenderNode(const ORenderNode& rhs) = delete;
	ORenderNode& operator=(const ORenderNode& rhs) = delete;
	virtual ~ORenderNode() = default;

	virtual void Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue, ORenderGraph* OtherParentGraph, const SPSOType& Type);
	virtual ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget);
	virtual void SetupCommonResources();
	const SNodeInfo& GetNodeInfo() const { return NodeInfo; }
	auto GetNextNode() const { return NodeInfo.NextNode; }
	void SetPSO(const SPSOType& PSOType) const;
	SPSODescriptionBase* FindPSOInfo(SPSOType Name) const;
	void SetNodeEnabled(bool bEnable);
	virtual void Update() {}

protected:
	OCommandQueue* CommandQueue = nullptr;
	SPSOType PSO;

private:
	SNodeInfo NodeInfo;
	ORenderGraph* ParentGraph;
};
