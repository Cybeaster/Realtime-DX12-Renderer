#pragma once
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "PSOReader/PsoReader.h"
#include "ShaderReader/ShaderReader.h"
#include "Types.h"

struct SRootSignature
{
	string Name;
	ComPtr<ID3D12RootSignature> RootSignature;
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
};

class OGraphicsPipelineManager
{
	using SGlobalShaderPipelineMap = unordered_map<string, SShadersPipeline>;
	using SGlobalShaderMap = unordered_map<string, unordered_map<EShaderLevel, unique_ptr<OShader>>>;
	using SGlobalPSOMap = unordered_map<string, unique_ptr<SPSODescriptionBase>>;

public:
	void Init();
	SPSODescriptionBase* FindPSO(const string& PipelineName);
	SShadersPipeline* FindShadersPipeline(const string& PipelineName);
	OShader* FindShader(const string& PipelineName, EShaderLevel ShaderType);

protected:
	void LoadShaders();
	void LoadPipelines();
	void LoadRenderNodes();
	void PutShaderContainer(const string& PipelineName, vector<unique_ptr<OShader>>& Shaders);
	SShadersPipeline MakePipelineInfoForPSO(const shared_ptr<SPSODescriptionBase>& PSO);
	shared_ptr<SShaderPipelineDesc> FindRootSignatureForPipeline(const string& PipelineName);
	unique_ptr<OShaderReader> ShaderReader;
	unique_ptr<OPSOReader> PSOReader;
	SGlobalShaderPipelineMap GlobalShaderPipelineMap;
	SGlobalShaderMap GlobalShaderMap;
	SGlobalPSOMap GlobalPSOMap;
	unordered_map<string, shared_ptr<SShaderPipelineDesc>> RootSignatures;
};

inline bool operator==(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Lhs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Rhs)
{
	return Lhs.Desc_1_1.Flags == Rhs.Desc_1_1.Flags
	       && Lhs.Desc_1_1.NumParameters == Rhs.Desc_1_1.NumParameters
	       && Lhs.Desc_1_1.NumStaticSamplers == Rhs.Desc_1_1.NumStaticSamplers
	       && Lhs.Desc_1_1.pParameters == Rhs.Desc_1_1.pParameters
	       && Lhs.Version == Rhs.Version;
}