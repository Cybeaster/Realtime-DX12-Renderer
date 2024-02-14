#include "DirectXUtils.h"

#include "Logger.h"

UINT Utils::CalcBufferByteSize(const UINT ByteSize)
{
	return (ByteSize + 255) & ~255;
}

ComPtr<ID3DBlob> Utils::CompileShader(const std::wstring& FileName, const D3D_SHADER_MACRO* Defines, const std::string& EntryPoint, const std::string& Target)
{
	UINT compileFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(FileName.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint.c_str(), Target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
	{
		OutputDebugStringA(static_cast<char*>(errors->GetBufferPointer()));
	}
	THROW_IF_FAILED(hr);
	return byteCode;
}

ComPtr<ID3DBlob> Utils::LoadBinary(const wstring& FileName)
{
	std::fstream fIn(FileName, std::ios::in | std::ios::binary);
	fIn.seekg(0, std::ios_base::end);
	std::fstream::pos_type size = static_cast<int>(fIn.tellg());
	fIn.seekg(0, std::ios_base::beg);

	ComPtr<ID3DBlob> blob;
	THROW_IF_FAILED(D3DCreateBlob(size, blob.GetAddressOf()));

	fIn.read(static_cast<char*>(blob->GetBufferPointer()), size);
	fIn.close();
	return blob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Utils::CreateDefaultBuffer(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList, const void* InitData, UINT64 ByteSize, ComPtr<ID3D12Resource>& UploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	const auto property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const auto buffer = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
	// Create the actual default buffer resource.
	THROW_IF_FAILED(Device->CreateCommittedResource(&property,
	                                                D3D12_HEAP_FLAG_NONE,
	                                                &buffer,
	                                                D3D12_RESOURCE_STATE_COMMON,
	                                                nullptr,
	                                                IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	const auto uploadProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap.
	THROW_IF_FAILED(Device->CreateCommittedResource(
	    &uploadProperty,
	    D3D12_HEAP_FLAG_NONE,
	    &buffer,
	    D3D12_RESOURCE_STATE_GENERIC_READ,
	    nullptr,
	    IID_PPV_ARGS(UploadBuffer.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = InitData;
	subResourceData.RowPitch = ByteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	auto commonCpyTransition = CD3DX12_RESOURCE_BARRIER::Transition(
	    defaultBuffer.Get(),
	    D3D12_RESOURCE_STATE_COMMON,
	    D3D12_RESOURCE_STATE_COPY_DEST);

	auto cpyDestGenericReadTransition = CD3DX12_RESOURCE_BARRIER::Transition(
	    defaultBuffer.Get(),
	    D3D12_RESOURCE_STATE_COPY_DEST,
	    D3D12_RESOURCE_STATE_GENERIC_READ);
	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	CommandList->ResourceBarrier(1, &commonCpyTransition);

	UpdateSubresources(CommandList, defaultBuffer.Get(), UploadBuffer.Get(), 0, 0, 1, &subResourceData);

	CommandList->ResourceBarrier(1, &cpyDestGenericReadTransition);

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.
	return defaultBuffer;
}
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

static map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStateMap = {};

D3D12_RESOURCE_STATES Utils::ResourceBarrier(ID3D12GraphicsCommandList* CMDList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES After)
{
	static map<ID3D12Resource*, D3D12_RESOURCE_STATES> ResourceStateMap = {};

	D3D12_RESOURCE_STATES localBefore = {};
	if (ResourceStateMap.contains(Resource))
	{
		localBefore = ResourceStateMap[Resource];
	}
	ResourceStateMap[Resource] = After;

	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, localBefore, After);
	CMDList->ResourceBarrier(1, &barrier);
	return localBefore;
}

void Utils::ResourceBarrier(ID3D12GraphicsCommandList* CMDList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
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
		LOG(Debug, Warning, "ResourceBarrier: Resource state mismatch on resource");
	}

	if (localBefore == After)
	{
		LOG(Debug, Warning, "ResourceBarrier: Resource states must be different!");
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