#pragma once
#include "Camera/Camera.h"
#include "Engine/RenderTarget/CubeMap/CubeRenderTarget.h"

class ODynamicCubeMapRenderTarget final : public OCubeRenderTarget
{
public:
	ODynamicCubeMapRenderTarget(const SRenderTargetParams& Params, DirectX::XMFLOAT3 Center, const DirectX::XMUINT2& Res)
	    : OCubeRenderTarget(Params, Res), Position(Center) {}

	void InitRenderObject() override;
	void UpdatePass(const SPassConstantsData& Data) override;
	string GetName() override
	{
		return "DynamicCubeMapTarget";
	}

private:
	DirectX::XMFLOAT3 Position;
	vector<unique_ptr<OCamera>> Cameras;
};
