#pragma once
#include "DXHelper.h"
#include "Logger.h"
#include "RenderConstants.h"
#include "Types.h"
struct SRootSignatureParams;
struct SShadersPipeline;
struct SShaderPipelineDesc;
using SGraphicsPSODesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC;
using SComputePSODesc = D3D12_COMPUTE_PIPELINE_STATE_DESC;

enum class EShaderLevel
{
	VertexShader,
	PixelShader,
	GeometryShader,
	HullShader,
	DomainShader,
	ComputeShader,
	//RT shaders
	RayGen,
	Miss,
	ClosestHit,
	AnyHit,
	Intersection,

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
	SPSOType Name;
	string RootSignatureName;
	SShaderArrayText ShaderPipeline;
	shared_ptr<SShaderPipelineDesc> RootSignature;
	ComPtr<ID3D12PipelineState> PSO;

	virtual bool BuildPipelineState(ID3D12Device* Device) = 0;

	virtual void SetVertexByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
	virtual void SetPixelByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
	virtual void SetGeometryByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
	virtual void SetHullByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
	virtual void SetDomainByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
	virtual void SetComputeByteCode(const D3D12_SHADER_BYTECODE& ByteCode) {}
};

struct SPSOGraphicsDescription : SPSODescriptionBase
{
	SGraphicsPSODesc PSODesc;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveTopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bool BuildPipelineState(ID3D12Device* Device) override;
	void SetVertexByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.VS = ByteCode; }
	void SetPixelByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.PS = ByteCode; }
	void SetGeometryByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.GS = ByteCode; }
	void SetHullByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.HS = ByteCode; }
	void SetDomainByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.DS = ByteCode; }
};

struct SPSOComputeDescription : SPSODescriptionBase
{
	SComputePSODesc PSODesc;
	void SetComputeByteCode(const D3D12_SHADER_BYTECODE& ByteCode) override { PSODesc.CS = ByteCode; }
	bool BuildPipelineState(ID3D12Device* Device) override;
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
	friend SShaderPipelineDesc;
	unordered_set<wstring> RootParamNames{};
	unordered_map<wstring, SRootParameter> RootParamMap{};
	unordered_map<wstring, uint32_t> RootParamIndexMap{};
	vector<D3D12_DESCRIPTOR_RANGE1> DescriptorRanges{};
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc{};
	ComPtr<ID3D12RootSignature> RootSignature;

private:
	vector<D3D12_ROOT_PARAMETER1> RootParameters{};
};

struct SShaderPipelineDesc
{
	SShaderPipelineDesc() = default;
	SShaderPipelineDesc(const SShaderPipelineDesc&) = default;
	EPSOType Type;
	string PipelineName = "NONE";
	SRootSignatureParams RootSignatureParams;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	vector<string> InputElementSemanticNames;
	vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;

	void AddRootParameter(const D3D12_ROOT_PARAMETER1& RootParameter, const wstring& Name);
	bool TryAddRootParameterName(const wstring& Name);

	void SetResource(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList);
	void SetResource(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Handle, ID3D12GraphicsCommandList* CmdList);

	void ActivateRootSignature(ID3D12GraphicsCommandList* CmdList) const;

	unordered_map<wstring, uint32_t>& GetRootParamIndexMap();
	int32_t GetIndexFromName(const string& Name);
	SRootParameter& GetRootParameterFromName(const string& Name);
	vector<D3D12_ROOT_PARAMETER1>& BuildParameterArray();

private:
	void SetComputeResourceCBView(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList);
	void SetGraphicsResourceCBView(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList);
};

struct SShaderMacro
{
	string Name;
	string Definition;
};

struct SPipelineStage
{
	wstring ShaderPath;
	string ShaderName;
	SShaderDefinition ShaderDefinition;
	vector<SShaderMacro> Defines;
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
	shared_ptr<SShaderPipelineDesc> PipelineInfo;

	void BuildFromStages(const vector<SPipelineStage>& Stages);
	ID3D12RootSignature* GetRootSignature() const;
};

D3D12_SHADER_VISIBILITY ShaderTypeToVisibility(EShaderLevel ShaderType);