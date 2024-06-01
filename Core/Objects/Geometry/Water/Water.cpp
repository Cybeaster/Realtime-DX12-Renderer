

#include "Water.h"

#include "EngineHelper.h"
void OWaterRenderObject::InitRenderObject()
{
	SRenderItemParams params;
	params.NumberOfInstances = 1;
	params.bFrustumCoolingEnabled = false;
	params.Pickable = false;
	params.Material = FindMaterial("Water01");
	params.OverrideLayer = SRenderLayers::Water;
	WaterItem = CreateGridRenderItem(
	    "Water",
	    1024,
	    1024,
	    2048,
	    2048,
	    params);
}
void OWaterRenderObject::Update(const UpdateEventArgs& Event)
{
	/*	for (auto& inst : WaterItem->Instances)
	{
		auto timer = Event.Timer;
		const auto mat = FindMaterial(inst.MaterialIndex);
		float& tu = mat->MatTransform(3, 0);
		float& tv = mat->MatTransform(3, 1);
		tu += 0.1f * timer.GetDeltaTime();
		tv += 0.02f * timer.GetDeltaTime();

		if (tu >= 1.0)
		{
			tu -= 1.0f;
		}

		if (tv >= 1.0)
		{
			tv -= 1.0f;
		}

		mat->MatTransform(3, 0) = tu;
		mat->MatTransform(3, 1) = tv;
		mat->NumFramesDirty = SRenderConstants::NumFrameResources;
	}*/
}