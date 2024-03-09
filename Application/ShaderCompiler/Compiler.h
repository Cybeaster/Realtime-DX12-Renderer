#pragma once
#include "Engine/Shader/Shader.h"
#include "Types.h"

#include <d3d12shader.h> // Contains functions and structures useful in accessing shader information.
#include <dxcapi.h>

struct SShaderPipelineDesc;
class OShaderCompiler
{
public:
	unique_ptr<OShader> CompileShader(const SShaderDefinition& Definition, const wstring& ShaderPath, SShaderPipelineDesc& OutPipelineInfo);
	vector<unique_ptr<OShader>> CompileShaders(vector<SPipelineStage>& OutPipelines, SShaderPipelineDesc& OutShadersPipeline);
	void Init();

private:
	ComPtr<ID3D12RootSignature> BuildRootSignature(vector<D3D12_ROOT_PARAMETER1>& RootParameter, const vector<CD3DX12_STATIC_SAMPLER_DESC>& StaticSamplers, D3D12_VERSIONED_ROOT_SIGNATURE_DESC& OutDescription);
	D3D12_SHADER_DESC BuildReflection(DxcBuffer Buffer, ComPtr<ID3D12ShaderReflection>& OutReflection);
	void GetInputLayoutDesc(const ComPtr<ID3D12ShaderReflection>& Reflection, SShaderPipelineDesc& OutPipelineInfo);
	void SetCompilationArgs(const SShaderDefinition& Definition);
	void ResolveBoundResources(const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_DESC& ShaderDescription, SShaderPipelineDesc& OutPipelineInfo, EShaderLevel ShaderType);
	void ResolveConstantBuffers(int32_t ResourceIdx, const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SShaderPipelineDesc& OutPipelineInfo);
	void ResolveTexturesAndStructuredBuffers(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SShaderPipelineDesc& OutPipelineInfo, EShaderLevel ShaderType);
	D3D12_DESCRIPTOR_RANGE_TYPE GetRangeType(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc);
	std::tuple<DxcBuffer, ComPtr<IDxcResult>> CreateDxcBuffer(const wstring& ShaderPath);
	ComPtr<IDxcCompiler3> Compiler;
	ComPtr<IDxcUtils> Utils;
	ComPtr<IDxcIncludeHandler> IncludeHandler;
	vector<LPCWSTR> CompilationArgs;
	wstring Entry;
	wstring TargetProfile;
};
