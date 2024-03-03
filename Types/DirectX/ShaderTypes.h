#pragma once
#include "Types.h"
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

struct SRootSignatureParams
{
	unordered_set<wstring> RootParamNames{};
	vector<D3D12_ROOT_PARAMETER1> RootParameters{};
	unordered_map<wstring, D3D12_ROOT_PARAMETER1> RootParamMap{};
	vector<D3D12_DESCRIPTOR_RANGE1> DescriptorRanges{};
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc{};
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

	void AddRootParameter(const D3D12_ROOT_PARAMETER1& RootParameter)
	{
		RootSignatureParams.RootParameters.push_back(RootParameter);
	}

	bool TryAddRootParameterName(const wstring& Name)
	{
		if (RootSignatureParams.RootParamNames.contains(Name))
		{
			LOG(Engine, Log, "Root parameter name already exists: {}", Name);
			RootSignatureParams.RootParamMap[Name].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			return false;
		}
		RootSignatureParams.RootParamNames.insert(Name);
		return true;
	}
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

	void BuildFromStages(const vector<SPipelineStage>& Stages)
	{
		for (auto Stage : Stages)
		{
			switch (Stage.ShaderDefinition.ShaderType)
			{
			case EShaderLevel::VertexShader:
				VertexShader = std::move(Stage);
				break;
			case EShaderLevel::PixelShader:
				PixelShader = std::move(Stage);
				break;
			case EShaderLevel::GeometryShader:
				GeometryShader = std::move(Stage);
				break;
			case EShaderLevel::HullShader:
				HullShader = std::move(Stage);
				break;
			case EShaderLevel::DomainShader:
				DomainShader = std::move(Stage);
				break;
			case EShaderLevel::ComputeShader:
				ComputeShader = std::move(Stage);
				break;
			}
		}
	}
};

inline D3D12_SHADER_VISIBILITY ShaderTypeToVisibility(EShaderLevel ShaderType)
{
	switch (ShaderType)
	{
	case EShaderLevel::VertexShader:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case EShaderLevel::PixelShader:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	case EShaderLevel::GeometryShader:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case EShaderLevel::HullShader:
		return D3D12_SHADER_VISIBILITY_HULL;
	case EShaderLevel::DomainShader:
		return D3D12_SHADER_VISIBILITY_DOMAIN;
	case EShaderLevel::ComputeShader:
		return D3D12_SHADER_VISIBILITY_ALL;
	}
	return D3D12_SHADER_VISIBILITY_ALL;
}