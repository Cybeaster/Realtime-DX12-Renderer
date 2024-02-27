//
// Created by Cybea on 27/02/2024.
//

#include "Shader.h"

void OShader::Init(const SShaderInfo& Info, const ComPtr<IDxcBlob>& Blob, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Desc)
{
	ShaderBlob = Blob;
	RootSignatureDesc = Desc;
	ShaderInfo = Info;
}
