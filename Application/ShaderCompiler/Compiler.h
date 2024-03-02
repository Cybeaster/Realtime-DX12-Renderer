#pragma once
#include "DXHelper.h"
#include "Engine/Shader/Shader.h"
#include "Types.h"

#include <d3d12shader.h> // Contains functions and structures useful in accessing shader information.
#include <dxcapi.h>

struct SPipelineInfo;
class OShaderCompiler
{
public:
	unique_ptr<OShader> CompileShader(const SShaderDefinition& Definition, wstring ShaderPath, SPipelineInfo& OutPipelineInfo);
	vector<unique_ptr<OShader>> CompileShaders(const vector<SPipelineStage>& OutPipelines, SPipelineInfo& OutShadersPipeline);
	void Init();

private:
	ComPtr<ID3D12RootSignature> BuildRootSignature(vector<D3D12_ROOT_PARAMETER1>& RootParameter, const vector<CD3DX12_STATIC_SAMPLER_DESC>& StaticSamplers, D3D12_VERSIONED_ROOT_SIGNATURE_DESC& OutDescription);
	D3D12_SHADER_DESC BuildReflection(DxcBuffer Buffer, ComPtr<ID3D12ShaderReflection>& OutReflection);
	D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc(const ComPtr<ID3D12ShaderReflection>& Reflection);
	void SetCompilationArgs(const SShaderDefinition& Definition);
	void ResolveBoundResources(const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_DESC& ShaderDescription, SPipelineInfo& OutPipelineInfo);
	void ResolveConstantBuffers(int32_t ResourceIdx, const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SPipelineInfo& OutPipelineInfo);
	void ResolveTexturesAndStructuredBuffers(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SPipelineInfo& OutPipelineInfo);
	std::tuple<DxcBuffer, ComPtr<IDxcResult>> CreateDxcBuffer(const wstring& ShaderPath);
	ComPtr<IDxcCompiler3> Compiler;
	ComPtr<IDxcUtils> Utils;
	ComPtr<IDxcIncludeHandler> IncludeHandler;
	vector<LPCWSTR> CompilationArgs;
	wstring Entry;
	wstring TargetProfile;
};
