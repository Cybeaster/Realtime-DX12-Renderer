#include "DirectX.h"

array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Utils::GetStaticSamplers()
{
	//clang-format off
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
	    0, // shaderRegister
	    D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
	    1, // shaderRegister
	    D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
	    2, // shaderRegister
	    D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
	    3, // shaderRegister
	    D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
	    4, // shaderRegister
	    D3D12_FILTER_ANISOTROPIC, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW
	    0.0f, // mipLODBias
	    8); // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
	    5, // shaderRegister
	    D3D12_FILTER_ANISOTROPIC, // filter
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
	    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW
	    0.0f, // mipLODBias
	    8); // maxAnisotropy

	return {
		pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp
	};
	//clang-format on
}
void Utils::ResourceBarrier(ID3D12GraphicsCommandList* CMDList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
	static map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStateMap = {};

	D3D12_RESOURCE_STATES localBefore = {};
	if (ResourceStateMap.contains(Resource))
	{
		localBefore = ResourceStateMap[Resource];
	}
	else
	{
		localBefore = Before;
	}

	ResourceStateMap[Resource] = After;

	if (localBefore != Before)
	{
		LOG(Warning, "ResourceBarrier: Resource state mismatch on resource");
	}

	if (localBefore == After)
	{
		LOG(Warning, "ResourceBarrier: Resource states must be different!");
		return;
	}

	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, localBefore, After);
	CMDList->ResourceBarrier(1, &barrier);
}

void Utils::BuildRootSignature(ID3D12Device* Device, ComPtr<ID3D12RootSignature>& RootSignature, const D3D12_ROOT_SIGNATURE_DESC& Desc)
{
	// Create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&Desc,
	                                         D3D_ROOT_SIGNATURE_VERSION_1,
	                                         serializedRootSig.GetAddressOf(),
	                                         errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
	}
	THROW_IF_FAILED(hr);
	THROW_IF_FAILED(Device->CreateRootSignature(0,
	                                            serializedRootSig->GetBufferPointer(),
	                                            serializedRootSig->GetBufferSize(),
	                                            IID_PPV_ARGS(&RootSignature)));
}