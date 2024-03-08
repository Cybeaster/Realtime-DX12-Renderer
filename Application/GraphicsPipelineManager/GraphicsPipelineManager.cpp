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
		LOG(Render, Log, "Loading PSO: {}", TEXT(pso->Name));
		if (pso->RootSignatureName.empty())
		{
			LOG(Render, Error, "Root signature not found for PSO: {}", TEXT(pso->Name));
			continue;
		}

		if (auto vertex = FindShader(pso->ShaderPipeline.VertexShaderName, EShaderLevel::VertexShader))
		{
			pso->SetVertexByteCode(vertex->GetShaderByteCode());
		}
		if (auto pixel = FindShader(pso->ShaderPipeline.PixelShaderName, EShaderLevel::PixelShader))
		{
			pso->SetPixelByteCode(pixel->GetShaderByteCode());
		}
		if (auto geometry = FindShader(pso->ShaderPipeline.GeometryShaderName, EShaderLevel::GeometryShader))
		{
			pso->SetGeometryByteCode(geometry->GetShaderByteCode());
		}
		if (auto hull = FindShader(pso->ShaderPipeline.HullShaderName, EShaderLevel::HullShader))
		{
			pso->SetHullByteCode(hull->GetShaderByteCode());
		}
		if (auto domain = FindShader(pso->ShaderPipeline.DomainShaderName, EShaderLevel::DomainShader))
		{
			pso->SetDomainByteCode(domain->GetShaderByteCode());
		}
		if (auto compute = FindShader(pso->ShaderPipeline.ComputeShaderName, EShaderLevel::ComputeShader))
		{
			pso->SetComputeByteCode(compute->GetShaderByteCode());
		}

		pso->RootSignature = FindRootSignatureForPipeline(pso->RootSignatureName);
		pso->BuildPipelineState(OEngine::Get()->GetDevice().Get());
	}
}

OShader* OGraphicsPipelineManager::FindShader(const string& PipelineName, EShaderLevel ShaderType)
{
	if (PipelineName.empty())
	{
		return nullptr;
	}

	if (!GlobalShaderMap.contains(PipelineName))
	{
		LOG(Render, Error, "Shader not found: {}", TEXT(PipelineName));
		return nullptr;
	}
	return GlobalShaderMap[PipelineName][ShaderType].get();
}

void OGraphicsPipelineManager::Init()
{
	LoadShaders();
	LoadPipelines();
}

SPSODescriptionBase* OGraphicsPipelineManager::FindPSO(const string& PipelineName)
{
	if (GlobalPSOMap.contains(PipelineName))
	{
		return GlobalPSOMap[PipelineName].get();
	}
	LOG(Render, Warning, "Pipeline not found: {}", TEXT(PipelineName));
	return nullptr;
}

SShadersPipeline* OGraphicsPipelineManager::FindShadersPipeline(const string& PipelineName)
{
	if (GlobalShaderPipelineMap.contains(PipelineName))
	{
		return &GlobalShaderPipelineMap[PipelineName];
	}
	LOG(Render, Warning, "Pipeline not found: {}", TEXT(PipelineName));
	return nullptr;
}

void OGraphicsPipelineManager::LoadShaders()
{
	ShaderReader = make_unique<OShaderReader>(OApplication::Get()->GetConfigPath("ShadersConfigPath"));
	auto pipelines = ShaderReader->LoadShaders();
	for (auto& pipeline : pipelines)
	{
		const auto newSignature = make_shared<SShaderPipelineDesc>();
		auto shaders = OEngine::Get()->GetShaderCompiler()->CompileShaders(pipeline.second, *newSignature);
		newSignature->PipelineName = pipeline.first;
		SShadersPipeline shadersPipeline;
		shadersPipeline.BuildFromStages(pipeline.second);
		shadersPipeline.PipelineInfo = newSignature;

		GlobalShaderPipelineMap[pipeline.first] = shadersPipeline;
		RootSignatures[pipeline.first] = newSignature;
		PutShaderContainer(pipeline.first, shaders);
	}
}

void OGraphicsPipelineManager::PutShaderContainer(const string& PipelineName, vector<unique_ptr<OShader>>& Shaders)
{
	for (auto& shader : Shaders)
	{
		GlobalShaderMap[PipelineName][shader->GetShaderType()] = std::move(shader);
	}
}

shared_ptr<SShaderPipelineDesc> OGraphicsPipelineManager::FindRootSignatureForPipeline(const string& PipelineName)
{
	if (RootSignatures.contains(PipelineName)) // TODO may be wrong with multiple signatures
	{
		return RootSignatures[PipelineName];
	}
	LOG(Render, Warning, "Pipeline not found: {}", TEXT(PipelineName));
	return nullptr;
}
