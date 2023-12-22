
#include "Camera.h"

#include "Window/Window.h"

OCamera::OCamera(DirectX::XMVECTOR _Position, DirectX::XMVECTOR _Target, shared_ptr<OWindow> _Window)
    : Position(_Position), Target(_Target)
{
	Init(_Window);
}

OCamera::OCamera(const shared_ptr<OWindow>& _Window)
{
	Init(_Window);
}

void OCamera::Init(const shared_ptr<OWindow>& _Window)
{
	Window = _Window;
	// ViewMatrix = DirectX::XMMatrixLookAtLH(Position, Target, Up);
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, _Window->GetAspectRatio(), 1.f, 1000.0f);
}
