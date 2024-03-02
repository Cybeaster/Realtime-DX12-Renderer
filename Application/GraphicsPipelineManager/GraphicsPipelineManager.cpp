//
// Created by Cybea on 28/02/2024.
//

#include "GraphicsPipelineManager.h"

#include "Application.h"

void OGraphicsPipelineManager::LoadPipelines()
{
	PSOReader = make_unique<OPSOReader>(OApplication::Get()->GetConfigPath("PSOConfigPath"));
	auto psos = PSOReader->LoadPSOs();
	for (auto& pso : psos)
	{
		auto pipeline = GetPipelineFor(pso->ShaderPipeline);
		if (const auto graphics = Cast<SPSODescription<SGraphicsPSODesc>>(pso.get()))
		{
			graphics->PSODesc.VS = pipeline.VertexShader.GetShaderByteCode();
			graphics->PSODesc.PS = pipeline.PixelShader.GetShaderByteCode();
			graphics->PSODesc.GS = pipeline.GeometryShader.GetShaderByteCode();
			graphics->PSODesc.HS = pipeline.HullShader.GetShaderByteCode();
			graphics->PSODesc.DS = pipeline.DomainShader.GetShaderByteCode();
		}
		else if (const auto compute = Cast<SPSODescription<SComputePSODesc>>(pso.get()))
		{
			compute->PSODesc.CS = pipeline.ComputeShader.GetShaderByteCode();
		}
		GlobalPipelineMap[pso->Name].PipelineInfo.PSODesc = pso;
	}
}

void OGraphicsPipelineManager::Init()
{
	LoadShaders();
	LoadPipelines();
}

void OGraphicsPipelineManager::LoadShaders()
{
	ShaderReader = make_unique<OShaderReader>(OApplication::Get()->GetConfigPath("ShadersConfigPath"));
	auto pipelines = ShaderReader->LoadShaders();
	SPipelineInfo newPipeline;
	for (auto& pipeline : pipelines)
	{
		auto shaders = OEngine::Get()->GetShaderCompiler()->CompileShaders(pipeline.second, newPipeline);
		SShadersPipeline shadersPipeline;
		shadersPipeline.BuildFromStages(pipeline.second);
		shadersPipeline.PipelineInfo = std::move(newPipeline);
		GlobalPipelineMap[pipeline.first] = std::move(shadersPipeline);
		PutShaderContainer(pipeline.first, shaders);
	}
}

void OGraphicsPipelineManager::PutShaderContainer(string PipelineName, vector<unique_ptr<OShader>>& Shaders)
{
	for (auto& shader : Shaders)
	{
		GlobalShaderMap[PipelineName][shader->GetShaderType()] = std::move(shader);
	}
}

SShadersPipeline OGraphicsPipelineManager::GetPipelineFor(const SShaderArrayText& ShaderArray)
{
	SShadersPipeline pipeline;
	pipeline.VertexShader = GlobalPipelineMap[ShaderArray.VertexShaderName].VertexShader;
	pipeline.PixelShader = GlobalPipelineMap[ShaderArray.PixelShaderName].PixelShader;
	pipeline.GeometryShader = GlobalPipelineMap[ShaderArray.GeometryShaderName].GeometryShader;
	pipeline.HullShader = GlobalPipelineMap[ShaderArray.HullShaderName].HullShader;
	pipeline.DomainShader = GlobalPipelineMap[ShaderArray.DomainShaderName].DomainShader;
	pipeline.ComputeShader = GlobalPipelineMap[ShaderArray.ComputeShaderName].ComputeShader;
	return pipeline;
}

SRootSignature* OGraphicsPipelineManager::FindRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& RootSignatureDesc)
{
	for (auto& rootSignature : RootSignatures | std::views::values)
	{
		if (rootSignature.RootSignatureDesc == RootSignatureDesc)
		{
			return &rootSignature;
		}
	}
	return nullptr;
}
