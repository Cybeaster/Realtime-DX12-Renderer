#include "RenderGraphReader.h"

#include <ranges>

vector<SNodeInfo> ORenderGraphReader::LoadRenderGraph()
{
	vector<SNodeInfo> result;
	for (auto& node : GetRootChild("RenderGraph") | std::views::values)
	{
		SNodeInfo info;
		info.Name = node.get<string>("Name");
		info.PSOType = node.get<string>("PSO");
		info.NextNode = node.get<string>("NextNode");
		info.RenderLayer = node.get<string>("RenderLayer");
		result.push_back(info);
	}
	return result;
}