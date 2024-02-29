#pragma once
#include "DXHelper.h"
#include "Logger.h"
#include "ShaderTypes.h"
#include "Types.h"

#include <dxcapi.h>

struct SShaderDefinition
{
	void TypeFromString(const string& Other);

	EShaderLevel ShaderType;
	string TargetProfile;
	string ShaderEntry;
};

class OShader
{
public:
	void Init(const SShaderDefinition& Info, const ComPtr<IDxcBlob>& Blob, D3D12_INPUT_LAYOUT_DESC Desc);

private:
	SShaderDefinition ShaderInfo = {};
	ComPtr<IDxcBlob> ShaderBlob;
	D3D12_INPUT_LAYOUT_DESC LayoutDesc = {};
};
