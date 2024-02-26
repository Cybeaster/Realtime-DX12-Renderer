#pragma once
#include "DXHelper.h"
#include "Types.h"
struct SNodeInfo
{
};
class ORenderNode;

class ORenderGraph
{
	using ODependencyInfo = unordered_map<ORenderNode*, vector<pair<shared_ptr<SNodeInfo>, ORenderNode*>>>;

public:
private:
	vector<unique_ptr<ORenderNode>> Nodes;
	ODependencyInfo Graph;
};
