#pragma once
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
public:
	void LoadPipelines();
	void Init();

protected:
	SRootSignature* FindRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& RootSignatureDesc);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetOpaquePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetAlphaTestedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTransparentPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetShadowPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetReflectedPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetMirrorPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDebugPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetCompositePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetSkyPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetTreeSpritePSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetIcosahedronPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetWavesRenderPSODesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetHighlightPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetBilateralBlurPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesDisturbPSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetWavesUpdatePSODesc();
	D3D12_COMPUTE_PIPELINE_STATE_DESC GetSobelPSODesc();

	void LoadShaders();

	unique_ptr<OShaderReader> ShaderReader;
	map<string, unique_ptr<OGraphicsPipeline>> Pipelines;
	map<string, SShadersPipeline> ShadersPipelines;
	unordered_map<string, vector<ComPtr<ID3D12PipelineState>>> PipelineStates;
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