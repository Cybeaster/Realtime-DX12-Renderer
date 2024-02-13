#pragma once
#include "UI/Widget.h"

#include <DirectXMath.h>

class OGeometryTransformWidget : public IWidget
{
public:
	DECLARE_DELEGATE(SGeometryUpdate, void);

	OGeometryTransformWidget(DirectX::XMFLOAT3* OutPosition, DirectX::XMFLOAT3* OutRotation, DirectX::XMFLOAT3* OutScale)
	    : Position(OutPosition), Rotation(OutRotation), Scale(OutScale){};

	void Draw() override;

	void SetInstanceParameters(DirectX::XMFLOAT3* OutPosition, DirectX::XMFLOAT3* OutRotation, DirectX::XMFLOAT3* OutScale)
	{
		Position = OutPosition;
		Rotation = OutRotation;
		Scale = OutScale;
	}

	SGeometryUpdate& GetOnTransformUpdate()
	{
		return OnTransformUpdate;
	}

private:
	SGeometryUpdate OnTransformUpdate;

	DirectX::XMFLOAT3* Position = nullptr;
	DirectX::XMFLOAT3* Rotation = nullptr;
	DirectX::XMFLOAT3* Scale = nullptr;
};