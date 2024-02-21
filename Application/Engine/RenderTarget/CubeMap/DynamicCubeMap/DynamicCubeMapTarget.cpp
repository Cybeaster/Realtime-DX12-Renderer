//
// Created by Cybea on 21/02/2024.
//

#include "DynamicCubeMapTarget.h"
void ODynamicCubeMapRenderTarget::Init()
{
	using namespace DirectX;
	OCubeRenderTarget::Init();

	auto x = Position.x;
	auto y = Position.y;
	auto z = Position.z;

	XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] = {
		XMFLOAT3(x + 1.0f, y, z), // +X
		XMFLOAT3(x - 1.0f, y, z), // -X
		XMFLOAT3(x, y + 1.0f, z), // +Y
		XMFLOAT3(x, y - 1.0f, z), // -Y
		XMFLOAT3(x, y, z + 1.0f), // +Z
		XMFLOAT3(x, y, z - 1.0f) // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] = {
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f), // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f) // -Z
	};
	Cameras.resize(GetNumRTVRequired());
	for (int i = 0; i < GetNumRTVRequired(); ++i)
	{
		Cameras[i] = make_unique<OCamera>();
		Cameras[i]->LookAt(Position, targets[i], ups[i]);
		Cameras[i]->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		Cameras[i]->UpdateViewMatrix();
	}
}