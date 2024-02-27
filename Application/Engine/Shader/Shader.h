#pragma once
#include "DXHelper.h"
#include "ShaderTypes.h"
#include "Types.h"

#include <dxcapi.h>

struct SShaderInfo
{
	wstring ShaderPath;
	wstring EntryPoint;
	wstring TargetProfile;
	vector<D3D_SHADER_MACRO> Defines;
	EShaderLevel ShaderType;
	uint32_t NumDescriptor;
};

class OShader
{
public:
	void Init(const SShaderInfo& Info, const ComPtr<IDxcBlob>& Blob, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Desc);

private:
	SShaderInfo ShaderInfo = {};
	ComPtr<IDxcBlob> ShaderBlob;
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
};
