
#include "Camera.h"

#include "Window/Window.h"

OCamera::OCamera(DirectX::XMVECTOR _Position, DirectX::XMVECTOR _Target, shared_ptr<OWindow> _Window)
    : Position(_Position), Target(_Target), Window(_Window)
{
	Init();
}

OCamera::OCamera(shared_ptr<OWindow> _Window)
    : Window(_Window)
{
	Init();
}

void OCamera::Init()
{
	ViewMatrix = DirectX::XMMatrixLookAtLH(Position, Target, Up);
	ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(Window.lock()->GetFoV(), Window.lock()->GetAspectRatio(), 0.1f, 100.0f);
}
