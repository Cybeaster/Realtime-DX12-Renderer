#pragma once
#include "Test/Test.h"

class OCubeMapTest : public OTest
{
public:
	OCubeMapTest(const shared_ptr<OWindow>& Window)
	    : OTest(Window) {}

	bool Initialize() override;
	void OnUpdate(const UpdateEventArgs& Event) override;
	void OnRender(const UpdateEventArgs& Event) override;
	void BuildRenderItems();
	void DrawSceneToCubeMap();
};
