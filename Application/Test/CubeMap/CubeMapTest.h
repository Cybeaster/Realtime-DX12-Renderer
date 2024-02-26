#pragma once
#include "Test/Test.h"

class OGPUWave;
class OCubeMapTest : public OTest
{
public:
	OCubeMapTest(OWindow* Window)
	    : OTest(Window) {}

	bool Initialize() override;
	void OnUpdate(const UpdateEventArgs& Event) override;
	void OnRender(const UpdateEventArgs& Event) override;
	void AnimateSkull(const UpdateEventArgs& Event);
	void BuildRenderItems();
	void DrawSceneToCubeMap();

private:
	OGPUWave* Waves = nullptr;

	SInstanceData* SkullRitem = nullptr;
};
