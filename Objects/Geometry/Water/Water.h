#pragma once
#include "DirectX/InstanceData.h"
#include "Engine/RenderTarget/RenderObject/RenderObject.h"

struct ORenderItem;
class OWaterRenderObject : public ORenderObjectBase
{
public:
	void InitRenderObject() override;
	void Update(const UpdateEventArgs& Event) override;

private:
	ORenderItem* WaterItem;
};
