#include "GraphicsPipeline.h"

#include "../../Utils/EngineHelper.h"
#include "ShaderTypes.h"

void SPipelineInfo::BuildPipelineState(ID3D12Device* Device)
{
	if (PSODesc->Type == EPSOType::Graphics)
	{
		auto desc = static_pointer_cast<SPSODescription<SGraphicsPSODesc>>(PSODesc)->PSODesc;
		THROW_IF_FAILED(Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&PipelineState)));
	}
	else
	{
		auto desc = static_pointer_cast<SPSODescription<SComputePSODesc>>(PSODesc)->PSODesc;
		THROW_IF_FAILED(Device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&PipelineState)));
	}
}

void SPipelineInfo::AddRootParameter(const D3D12_ROOT_PARAMETER1& RootParameter, const wstring& Name)
{
	RootSignatureParams.RootParamMap[Name] = { RootParameter, Name, RootParameter.ParameterType };
	RootSignatureParams.RootParamNames.insert(Name);
}

bool SPipelineInfo::TryAddRootParameterName(const wstring& Name)
{
	if (RootSignatureParams.RootParamNames.contains(Name))
	{
		LOG(Engine, Log, "Root parameter name already exists: {}", Name);
		RootSignatureParams.RootParamMap[Name].RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		return false;
	}
	RootSignatureParams.RootParamNames.insert(Name);
	return true;
}

vector<D3D12_ROOT_PARAMETER1>& SPipelineInfo::BuildParameterArray()
{
	RootSignatureParams.RootParameters.clear();
	for (auto& param : RootSignatureParams.RootParamMap | std::views::values)
	{
		RootSignatureParams.RootParameters.push_back(param.RootParameter);
	}
	return RootSignatureParams.RootParameters;
}

D3D12_SHADER_BYTECODE SPipelineStage::GetShaderByteCode() const
{
	if (Shader)
	{
		return Shader->GetShaderByteCode();
	}
	return {};
}
void SShadersPipeline::BuildFromStages(const vector<SPipelineStage>& Stages)
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

void SShadersPipeline::SetResource(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Handle, ID3D12GraphicsCommandList* CmdList)
{
	auto idx = PipelineInfo.RootParamIndexMap[UTF8ToWString(Name)];
	auto& param = PipelineInfo.RootSignatureParams.RootParamMap[UTF8ToWString(Name)];
	switch (param.Type)
	{
	case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
	case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
		CmdList->SetGraphicsRootShaderResourceView(idx, Handle);
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		CmdList->SetGraphicsRootDescriptorTable(idx, Handle);
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		break;
	}
}

D3D12_SHADER_VISIBILITY ShaderTypeToVisibility(EShaderLevel ShaderType)
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
