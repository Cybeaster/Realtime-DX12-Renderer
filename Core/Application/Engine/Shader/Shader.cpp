//
// Created by Cybea on 27/02/2024.
//

#include "Shader.h"

void SShaderDefinition::TypeFromString(const string& Other)
{
	if (Other == "Vertex")
	{
		ShaderType = EShaderLevel::VertexShader;
	}
	else if (Other == "Pixel")
	{
		ShaderType = EShaderLevel::PixelShader;
	}
	else if (Other == "Compute")
	{
		ShaderType = EShaderLevel::ComputeShader;
	}
	else if (Other == "Geometry")
	{
		ShaderType = EShaderLevel::GeometryShader;
	}
	else if (Other == "Hull")
	{
		ShaderType = EShaderLevel::HullShader;
	}
	else if (Other == "Domain")
	{
		ShaderType = EShaderLevel::DomainShader;
	}
	else if (Other == "RayGen")
	{
		ShaderType = EShaderLevel::RayGen;
	}
	else if (Other == "Miss")
	{
		ShaderType = EShaderLevel::Miss;
	}
	else if (Other == "ClosestHit")
	{
		ShaderType = EShaderLevel::ClosestHit;
	}
	else if (Other == "AnyHit")
	{
		ShaderType = EShaderLevel::AnyHit;
	}
	else if (Other == "Intersection")
	{
		ShaderType = EShaderLevel::Intersection;
	}
	else
	{
		WIN_LOG(Default, Error, "Unknown shader type: {}", TEXT(Other));
	}
}

void OShader::Init(const SShaderDefinition& Info, const ComPtr<IDxcBlob>& Blob)
{
	ShaderBlob = Blob;
	ShaderInfo = Info;
}

D3D12_SHADER_BYTECODE OShader::GetShaderByteCode() const
{
	return {
		reinterpret_cast<BYTE*>(ShaderBlob->GetBufferPointer()), ShaderBlob->GetBufferSize()
	};
}