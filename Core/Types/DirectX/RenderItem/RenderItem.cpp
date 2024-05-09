
#include "RenderItem.h"

#include "Engine/Engine.h"

void ORenderItem::Update(const UpdateEventArgs& Arg) const
{
	for (auto& Component : Components)
	{
		Component->Tick(Arg);
	}
}

bool ORenderItem::IsValid() const
{
	return RenderLayer != "NONE" && Geometry != nullptr;
}

bool ORenderItem::IsValidChecked() const
{
	const bool bIsValid = IsValid();
	CWIN_LOG(!bIsValid, Geometry, Error, "RenderLayer is NONE");
	return bIsValid;
}
HLSL::InstanceData* ORenderItem::GetDefaultInstance()
{
	return &Instances[0];
}

const vector<unique_ptr<OComponentBase>>& ORenderItem::GetComponents() const
{
	return Components;
}

void ORenderItem::OnMaterialChanged()
{
	auto old = RenderLayer;
	RenderLayer = DefaultMaterial->RenderLayer;
	OEngine::Get()->MoveRIToNewLayer(this, RenderLayer, old);
}

void SRenderItemParams::SetPosition(DirectX::XMFLOAT3 P)
{
	Position = P;
}

void SRenderItemParams::SetScale(DirectX::XMFLOAT3 S)
{
	Scale = S;
}