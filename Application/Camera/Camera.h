#pragma once
#include "Types.h"

#include <DirectXMath.h>
class OCamera
{
public:
	OCamera(DirectX::XMVECTOR _Position, DirectX::XMVECTOR _Target, shared_ptr<class OWindow> _Window);
	OCamera(const shared_ptr<OWindow>& _Window);
	OCamera() = default;

	void Init(const shared_ptr<OWindow>& _Window);
	DirectX::XMVECTOR Position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Target = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMFLOAT4X4 ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;

private:
	weak_ptr<class OWindow> Window;
};
