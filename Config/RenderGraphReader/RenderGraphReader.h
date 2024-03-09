
#pragma once
#include "ConfigReader.h"
#include "RenderNodeInfo.h"
class ORenderGraphReader : public OConfigReader
{
public:
	ORenderGraphReader(const string& FileName)
	    : OConfigReader(FileName) {}
	vector<SNodeInfo> LoadRenderGraph();
};
