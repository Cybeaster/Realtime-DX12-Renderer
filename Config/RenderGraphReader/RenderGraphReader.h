
#pragma once
#include "ConfigReader.h"
#include "RenderNodeInfo.h"
enum class EMaterialType : uint8_t;
class ORenderGraphReader : public OConfigReader
{
public:
	ORenderGraphReader(const string& FileName)
	    : OConfigReader(FileName) {}
	vector<SNodeInfo> LoadRenderGraph(string& OutHead);
};
