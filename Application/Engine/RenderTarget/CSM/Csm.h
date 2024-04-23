#pragma once
#include "Engine/RenderTarget/RenderTarget.h"
#include "Engine/RenderTarget/ShadowMap/ShadowMap.h"
class OCSM final : public ORenderTargetBase
{
public:
	OCSM(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, EResourceHeapType HeapType);

	void BuildDescriptors() override;
	void BuildResource() override;
	SResourceInfo* GetResource() override;

	array<DirectX::XMFLOAT3, 8> FrustrumCornens = {
		DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f),
		DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f),
		DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f),
		DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f)
	};

private:
	array<unique_ptr<OShadowMap>, 3> ShadowMaps;
};
