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
	virtual void Tick(UpdateEventArgs Arg){}
	string GetName() const;
protected:
	string Name = "ComponentBase";
	ORenderItem* Owner = nullptr;
};


class OSceneComponent : public OComponentBase
{
public:
	OSceneComponent(){Name = "SceneComponent";}
protected:
	DirectX::XMFLOAT4X4 WorldMatrix ={};
};


