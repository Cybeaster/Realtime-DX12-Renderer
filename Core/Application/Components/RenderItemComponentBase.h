#pragma once
#include "DirectX/DXHelper.h"
#include "Events.h"

struct ORenderItem;
struct SMaterial;
class OComponentBase
{
public:
	virtual ~OComponentBase() = default;
	virtual void Init(ORenderItem* Other);
	virtual void Tick(UpdateEventArgs Arg) {}
	string GetName() const;

protected:
	string Name = "ComponentBase";
	ORenderItem* Owner = nullptr;
};

class OSceneComponent : public OComponentBase
{
public:
	OSceneComponent() { Name = "SceneComponent"; }

protected:
	DirectX::XMFLOAT4X4 WorldMatrix = {};

	DirectX::XMFLOAT3 Position = {};
	DirectX::XMFLOAT3 Scale = { 1, 1, 1 };
	DirectX::XMFLOAT4 Rotation = {};
};
