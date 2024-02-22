#pragma once
#include "Camera/Camera.h"
#include "Engine/RenderTarget/CubeMap/CubeRenderTarget.h"

class ODynamicCubeMapRenderTarget final : public OCubeRenderTarget
{
public:
	ODynamicCubeMapRenderTarget(const SRenderTargetParams& Params, DirectX::XMFLOAT3 Center)
	    : OCubeRenderTarget(Params), Position(Center) {}

	virtual void Init() override;

private:
	DirectX::XMFLOAT3 Position;
	vector<unique_ptr<OCamera>> Cameras;
};
