#pragma once
#include "DXHelper.h"
#include "Logger.h"
#include "Types.h"
struct SPipelineInfo;
using SGraphicsPSODesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC;
using SComputePSODesc = D3D12_COMPUTE_PIPELINE_STATE_DESC;

enum class EShaderLevel
{
	VertexShader,
	PixelShader,
	GeometryShader,
	HullShader,
	DomainShader,
	ComputeShader
};

struct SShaderArrayText
{
	string VertexShaderName = "";
	string PixelShaderName = "";
	string GeometryShaderName = "";
	string HullShaderName = "";
	string DomainShaderName = "";
	string ComputeShaderName = "";
};

enum class EPSOType
{
	Graphics,
	Compute
};

struct SPSODescriptionBase
{
	virtual ~SPSODescriptionBase() {}
	EPSOType Type;
	string Name;
	SShaderArrayText ShaderPipeline;
};

template<typename T>
struct SPSODescription : SPSODescriptionBase
{
	T PSODesc;
};

struct SShaderDefinition
{
	void TypeFromString(const string& Other);

	EShaderLevel ShaderType;
	wstring TargetProfile;
	wstring ShaderEntry;
};

struct SRootParameter
{
	D3D12_ROOT_PARAMETER1 RootParameter;
	wstring Name;
	D3D12_ROOT_PARAMETER_TYPE Type;
};

struct SRootSignatureParams
{
	friend SPipelineInfo;
	unordered_set<wstring> RootParamNames{};
	unordered_map<wstring, SRootParameter> RootParamMap{};
	vector<D3D12_DESCRIPTOR_RANGE1> DescriptorRanges{};
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc{};

private:
	vector<D3D12_ROOT_PARAMETER1> RootParameters{};
};

struct SPipelineInfo
{
	SPipelineInfo() = default;
	SPipelineInfo(const SPipelineInfo&) = default;

	shared_ptr<SPSODescriptionBase> PSODesc;
	std::unordered_map<wstring, uint32_t> RootParamIndexMap{};
	SRootSignatureParams RootSignatureParams;
	ComPtr<ID3D12RootSignature> RootSignature;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{};
	ComPtr<ID3D12PipelineState> PipelineState;

	void BuildPipelineState(ID3D12Device* Device);
	void AddRootParameter(const D3D12_ROOT_PARAMETER1& RootParameter, const wstring& Name);
	bool TryAddRootParameterName(const wstring& Name);
	vector<D3D12_ROOT_PARAMETER1>& BuildParameterArray();
};

struct SPipelineStage
{
	wstring ShaderPath;
	string ShaderName;
	SShaderDefinition ShaderDefinition;
	vector<D3D_SHADER_MACRO> Defines;
	class OShader* Shader;
	D3D12_SHADER_BYTECODE GetShaderByteCode() const;
};

struct SShadersPipeline
{
	SPipelineStage VertexShader;
	SPipelineStage PixelShader;
	SPipelineStage GeometryShader;
	SPipelineStage HullShader;
	SPipelineStage DomainShader;
	SPipelineStage ComputeShader;
	SPipelineInfo PipelineInfo;
	void SetResource(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Handle, ID3D12GraphicsCommandList* CmdList);
	void BuildFromStages(const vector<SPipelineStage>& Stages);
};

inline D3D12_SHADER_VISIBILITY ShaderTypeToVisibility(EShaderLevel ShaderType);