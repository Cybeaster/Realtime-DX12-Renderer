#pragma once
#include "../../Config/PSOReader/PsoReader.h"
#include "../../Config/ShaderReader/ShaderReader.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "Types.h"

struct SRootSignature
{
	string Name;
	ComPtr<ID3D12RootSignature> RootSignature;
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
};

class OGraphicsPipelineManager
{
	using SGlobalPipelineMap = unordered_map<string, SShadersPipeline>;
	using SGlobalShaderMap = unordered_map<string, unordered_map<EShaderLevel, unique_ptr<OShader>>>;

public:
	void LoadPipelines();
	void Init();

protected:
	SRootSignature* FindRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& RootSignatureDesc);
	void PutShaderContainer(string PipelineName, vector<unique_ptr<OShader>>& Shaders);
	void LoadShaders();
	SShadersPipeline GetPipelineFor(const SShaderArrayText& ShaderArray);
	unique_ptr<OShaderReader> ShaderReader;
	unique_ptr<OPSOReader> PSOReader;
	SGlobalPipelineMap GlobalPipelineMap;
	SGlobalShaderMap GlobalShaderMap;
	unordered_map<string, SRootSignature> RootSignatures;
};

inline bool operator==(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Lhs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Rhs)
{
	return Lhs.Desc_1_1.Flags == Rhs.Desc_1_1.Flags
	       && Lhs.Desc_1_1.NumParameters == Rhs.Desc_1_1.NumParameters
	       && Lhs.Desc_1_1.NumStaticSamplers == Rhs.Desc_1_1.NumStaticSamplers
	       && Lhs.Desc_1_1.pParameters == Rhs.Desc_1_1.pParameters
	       && Lhs.Version == Rhs.Version;
}