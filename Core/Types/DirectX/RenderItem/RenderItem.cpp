
#include "RenderItem.h"

#include "Engine/Engine.h"

void ORenderItem::Update(const UpdateEventArgs& Arg)
{
	for (auto& Component : Components)
	{
		Component->Tick(Arg);
	}
}

bool ORenderItem::IsValid() const
{
	return RenderLayer != "NONE" && !Geometry.expired();
}

bool ORenderItem::IsValidChecked() const
{
	const bool bIsValid = IsValid();
	CWIN_LOG(!bIsValid, Geometry, Error, "RenderLayer is NONE");
	return bIsValid;
}

void ORenderItem::AddInstance(const SInstanceData& Instance)
{
	Instances.push_back(Instance);
}

SInstanceData* ORenderItem::GetDefaultInstance()
{
	if (Instances.empty())
	{
		return nullptr;
	}
	else
	{
		return &Instances[0];
	}
}

const vector<unique_ptr<OComponentBase>>& ORenderItem::GetComponents() const
{
	return Components;
}

void SRenderItemParams::SetPosition(DirectX::XMFLOAT3 P)
{
	Position = P;
}

void SRenderItemParams::SetScale(DirectX::XMFLOAT3 S)
{
	Scale = S;
}