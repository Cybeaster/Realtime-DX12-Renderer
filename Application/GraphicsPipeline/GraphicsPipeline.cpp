#include "GraphicsPipeline.h"

void SPSOGraphicsDescription::BuildPipelineState(ID3D12Device* Device)
{
	PSODesc.pRootSignature = RootSignature->RootSignatureParams.RootSignature.Get();
	PSODesc.InputLayout = {
		.pInputElementDescs = RootSignature->InputElementDescs.data(),
		.NumElements = static_cast<uint32_t>(RootSignature->InputElementDescs.size())
	};
	RootSignature->Type = EPSOType::Graphics;
	THROW_IF_FAILED(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));
}

void SPSOComputeDescription::BuildPipelineState(ID3D12Device* Device)
{
	PSODesc.pRootSignature = RootSignature->RootSignatureParams.RootSignature.Get();
	RootSignature->Type = EPSOType::Compute;
	THROW_IF_FAILED(Device->CreateComputePipelineState(&PSODesc, IID_PPV_ARGS(&PSO)));
}

void SShaderPipelineDesc::AddRootParameter(const D3D12_ROOT_PARAMETER1& RootParameter, const wstring& Name)
{
	RootSignatureParams.RootParamMap[Name] = { RootParameter, Name, RootParameter.ParameterType };
	RootSignatureParams.RootParamNames.insert(Name);
}

bool SShaderPipelineDesc::TryAddRootParameterName(const wstring& Name)
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

vector<D3D12_ROOT_PARAMETER1>& SShaderPipelineDesc::BuildParameterArray()
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
ID3D12RootSignature* SShadersPipeline::GetRootSignature() const
{
	return PipelineInfo->RootSignatureParams.RootSignature.Get();
}

void SShaderPipelineDesc::SetResourceCBView(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList)
{
	auto idx = GetIndexFromName(Name);
	auto& param = GetRootParameterFromName(Name);
	switch (param.Type)
	{
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		CmdList->SetGraphicsRootShaderResourceView(idx, Handle);
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		CmdList->SetGraphicsRootConstantBufferView(idx, Handle);
		break;
	}
}

void SShaderPipelineDesc::SetComputeResourceCBView(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList)
{
	auto idx = GetIndexFromName(Name);
	auto& param = GetRootParameterFromName(Name);
	switch (param.Type)
	{
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		CmdList->SetComputeRootShaderResourceView(idx, Handle);
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		CmdList->SetComputeRootConstantBufferView(idx, Handle);
		break;
	}
}

void SShaderPipelineDesc::SetGraphicsResourceCBView(const string& Name, D3D12_GPU_VIRTUAL_ADDRESS Handle, ID3D12GraphicsCommandList* CmdList)
{
	auto idx = GetIndexFromName(Name);
	auto& param = GetRootParameterFromName(Name);
	switch (param.Type)
	{
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		CmdList->SetGraphicsRootShaderResourceView(idx, Handle);
	case D3D12_ROOT_PARAMETER_TYPE_CBV:
		CmdList->SetGraphicsRootConstantBufferView(idx, Handle);
		break;
	}
}

void SShaderPipelineDesc::SetDescriptorTable(const string& Name, D3D12_GPU_DESCRIPTOR_HANDLE Handle, ID3D12GraphicsCommandList* CmdList)
{
	switch (Type)
	{
	case EPSOType::Graphics:
		CmdList->SetGraphicsRootDescriptorTable(GetIndexFromName(Name), Handle);
		break;
	case EPSOType::Compute:
		CmdList->SetComputeRootDescriptorTable(GetIndexFromName(Name), Handle);
		break;
	}
}

void SShaderPipelineDesc::ActivateRootSignature(ID3D12GraphicsCommandList* CmdList) const
{
	switch (Type)
	{
	case EPSOType::Graphics:
		CmdList->SetGraphicsRootSignature(RootSignatureParams.RootSignature.Get());

		break;
	case EPSOType::Compute:
		CmdList->SetComputeRootSignature(RootSignatureParams.RootSignature.Get());
		break;
	}
}

unordered_map<wstring, uint32_t>& SShaderPipelineDesc::GetRootParamIndexMap()
{
	return RootSignatureParams.RootParamIndexMap;
}

int32_t SShaderPipelineDesc::GetIndexFromName(const string& Name)
{
	if (!RootSignatureParams.RootParamIndexMap.contains(UTF8ToWString(Name)))
	{
		LOG(Render, Error, "Descriptor table name not found: {}", TEXT(Name));
	}
	return RootSignatureParams.RootParamIndexMap[UTF8ToWString(Name)];
}

SRootParameter& SShaderPipelineDesc::GetRootParameterFromName(const string& Name)
{
	if (!RootSignatureParams.RootParamMap.contains(UTF8ToWString(Name)))
	{
		LOG(Render, Error, "Descriptor table name not found: {}", TEXT(Name));
	}
	return RootSignatureParams.RootParamMap[UTF8ToWString(Name)];
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
