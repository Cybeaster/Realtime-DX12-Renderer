#pragma once
#include "Camera/Camera.h"
#include "Engine/RenderTarget/CubeMap/CubeRenderTarget.h"

class ODynamicCubeMapRenderTarget final : public OCubeRenderTarget
{
public:
	ODynamicCubeMapRenderTarget(const SRenderTargetParams& Params, const DirectX::XMUINT2& Res)
	    : OCubeRenderTarget(Params, Res) { Name = L"DynamicCubeMap"; }

	void InitRenderObject() override;
	void UpdatePass(const TUploadBufferData<SPassConstants>& Data) override;

	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddresss(int Index) const //TODO propagate to base class
	{
		auto& pass = PassConstants[Index];
		return pass.Buffer->GetGPUAddress() + pass.StartIndex * Utils::CalcBufferByteSize(sizeof(SPassConstants));
	}
	void SetBoundRenderItem(const shared_ptr<ORenderItem>& Item) override;
	void CalculateCameras();

private:
	DirectX::XMFLOAT3 Position;
	vector<unique_ptr<OCamera>> Cameras;
	vector<TUploadBufferData<SPassConstants>> PassConstants;
};
