#pragma once
#include "DXHelper.h"
#include "Types.h"
class ORenderNode
{
public:
	bool Initialize();
	bool Execute();


private:
	string RenderLayer;
};
