#pragma once
#include "DirectX/DXHelper.h"
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

	virtual void Initialize(const SNodeInfo& OtherNodeInfo, OCommandQueue* OtherCommandQueue, ORenderGraph* OtherParentGraph, SPSODescriptionBase* OtherPSO);
	virtual ORenderTargetBase* Execute(ORenderTargetBase* RenderTarget);
	virtual void SetupCommonResources();
	const SNodeInfo& GetNodeInfo() const { return NodeInfo; }
	auto GetNextNode() const { return NodeInfo.NextNode; }
	void SetPSO(const string& PSOType) const;
	SPSODescriptionBase* FindPSOInfo(string Name) const;
	void SetNodeEnabled(bool bEnable);

protected:
	OCommandQueue* CommandQueue = nullptr;
	SPSODescriptionBase* PSO = nullptr;

private:
	SNodeInfo NodeInfo;
	ORenderGraph* ParentGraph;
};
