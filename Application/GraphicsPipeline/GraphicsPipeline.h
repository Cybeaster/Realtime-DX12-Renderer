#pragma once
#include "DXHelper.h"
#include "Engine/Shader/Shader.h"
#include "Types.h"

struct SPipelineInfo
{
	SPipelineInfo() = default;
	SPipelineInfo(const SPipelineInfo&) = default;
	SPipelineInfo& operator=(SPipelineInfo&& Other) noexcept
	{
		RootParamIndexMap = std::move(Other.RootParamIndexMap);
		RootParameters = std::move(Other.RootParameters);
		DescriptorRanges = std::move(Other.DescriptorRanges);
		RootSignatureDesc = std::move(Other.RootSignatureDesc);
		return *this;
	}

	std::unordered_map<wstring, uint32_t> RootParamIndexMap{};
	vector<D3D12_ROOT_PARAMETER1> RootParameters{};
	vector<D3D12_DESCRIPTOR_RANGE1> DescriptorRanges{};
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc{};
	ComPtr<ID3D12RootSignature> RootSignature;
};

struct SShaderInfo
{
	wstring ShaderPath;
	string ShaderName;
	vector<D3D_SHADER_MACRO> Defines;
	vector<SShaderDefinition> Definitions;
	uint32_t NumDescriptor;
};

struct SShadersPipeline
{
	vector<unique_ptr<OShader>> Shaders;
	SPipelineInfo PipelineInfo;
};

class OGraphicsPipeline
{
public:
	void BuildShaders(const SShaderInfo& ShaderInfo);

private:
	ComPtr<ID3D12PipelineState> PipelineState;
	SPipelineInfo PipelineInfo;
	vector<OShader*> Shaders;
};
