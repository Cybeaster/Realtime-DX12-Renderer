#pragma once
#include "Test/Test.h"

class OWindowTest : public OTest
{
public:
	OWindowTest(const shared_ptr<OEngine>& _Engine, const shared_ptr<OWindow>& _Window)
	    : OTest(_Engine, _Window)
	{
	}

	void OnRender(const UpdateEventArgs& Arg) override;
};
