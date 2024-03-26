#include "RenderGraphReader.h"

#include <ranges>

vector<SNodeInfo> ORenderGraphReader::LoadRenderGraph(string& OutHead)
{
	vector<SNodeInfo> result;
	auto node = PTree.get_child("RenderGraph");
	OutHead = GetAttribute(node, "Head");
	for (auto& node : node.get_child("Nodes") | std::views::values)
	{
		SNodeInfo info;
		info.Name = node.get<string>("Name");
		info.PSOType = node.get<string>("PSO");
		info.NextNode = node.get<string>("NextNode");
		info.RenderLayer = node.get<string>("RenderLayer");
		info.bEnable = node.get<bool>("Enabled");
		result.push_back(info);
	}
	return result;
}