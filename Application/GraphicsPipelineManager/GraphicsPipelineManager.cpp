//
// Created by Cybea on 28/02/2024.
//

#include "GraphicsPipelineManager.h"

#include "Application.h"
void OGraphicsPipelineManager::LoadPipelines()
{
}

void OGraphicsPipelineManager::Init()
{
	LoadShaders();
}

void OGraphicsPipelineManager::LoadShaders()
{
	ShaderReader = make_unique<OShaderReader>(OApplication::Get()->GetConfigPath("ShadersConfigPath"));
	auto shadersInfo = ShaderReader->LoadShaders();
	SShadersPipeline newPipeline;
	SPipelineInfo pipelineInfo;
	for (auto& info : shadersInfo)
	{
		for (auto& val : info.Definitions)
		{
			auto newShader = OEngine::Get()->GetShaderCompiler()->CompilerShader(val, info.ShaderPath, pipelineInfo);
			newPipeline.Shaders.push_back(std::move(newShader));
		}
		ShadersPipelines[info.ShaderName] = std::move(newPipeline);
	}
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

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetOpaquePSODesc()
{
	UINT quality = 0;
	bool msaaEnable = GetMSAAState(quality);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePSO;
	ZeroMemory(&opaquePSO, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	opaquePSO.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
	opaquePSO.pRootSignature = DefaultRootSignature.Get();

	auto vsShader = GetShader(SShaderTypes::VSBaseShader);
	auto psShader = GetShader(SShaderTypes::PSOpaque);

	opaquePSO.VS = { reinterpret_cast<BYTE*>(vsShader->GetBufferPointer()), vsShader->GetBufferSize() };
	opaquePSO.PS = { reinterpret_cast<BYTE*>(psShader->GetBufferPointer()), psShader->GetBufferSize() };

	opaquePSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePSO.SampleMask = UINT_MAX;
	opaquePSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePSO.NumRenderTargets = 1;
	opaquePSO.RTVFormats[0] = SRenderConstants::BackBufferFormat;
	opaquePSO.SampleDesc.Count = msaaEnable ? 4 : 1;
	opaquePSO.SampleDesc.Quality = msaaEnable ? (quality - 1) : 0;
	opaquePSO.DSVFormat = SRenderConstants::DepthBufferFormat;
	return opaquePSO;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetAlphaTestedPSODesc()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPSO = GetOpaquePSODesc();
	const auto psAlphaTestedShader = GetShader(SShaderTypes::PSAlphaTested);
	alphaTestedPSO.PS = { reinterpret_cast<BYTE*>(psAlphaTestedShader->GetBufferPointer()), psAlphaTestedShader->GetBufferSize() };
	alphaTestedPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	return alphaTestedPSO;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetTransparentPSODesc()
{
	auto transparent = GetOpaquePSODesc();
	transparent.BlendState.RenderTarget[0] = GetTransparentBlendState();
	return transparent;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetShadowPSODesc()
{
	// We are going to draw shadows with transparency, so base it off the transparency description.
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = GetTransparentPSODesc();
	shadowPsoDesc.DepthStencilState = shadowDSS;
	return shadowPsoDesc;
}
D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetWavesRenderPSODesc()
{
	auto wavesRenderPSO = GetTransparentPSODesc();
	wavesRenderPSO.VS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::VSWaves)->GetBufferPointer()),
		GetShader(SShaderTypes::VSWaves)->GetBufferSize()
	};
	return wavesRenderPSO;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetHighlightPSODesc()
{
	auto highlightPSO = GetOpaquePSODesc();
	highlightPSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	highlightPSO.BlendState.RenderTarget[0] = GetTransparentBlendState();
	return highlightPSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetWavesDisturbPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesPSO = {};
	wavesPSO.pRootSignature = WavesRootSignature.Get();
	wavesPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSWavesDisturb)->GetBufferPointer()),
		GetShader(SShaderTypes::CSWavesDisturb)->GetBufferSize()
	};
	wavesPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return wavesPSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetWavesUpdatePSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesPSO = {};
	wavesPSO.pRootSignature = WavesRootSignature.Get();
	wavesPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSWavesUpdate)->GetBufferPointer()),
		GetShader(SShaderTypes::CSWavesUpdate)->GetBufferSize()
	};
	wavesPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return wavesPSO;
}
D3D12_COMPUTE_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetBilateralBlurPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC bilateralBlurPSO = {};
	bilateralBlurPSO.pRootSignature = BilateralBlurRootSignature.Get();
	bilateralBlurPSO.CS = GetShaderByteCode(SShaderTypes::CSBilateralBlur);
	bilateralBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return bilateralBlurPSO;
}

void OGraphicsPipelineManager::BuildBlurPSO()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC horizontalBlurPSO = {};
	horizontalBlurPSO.pRootSignature = BlurRootSignature.Get();
	horizontalBlurPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSHorizontalBlur)->GetBufferPointer()),
		GetShader(SShaderTypes::CSHorizontalBlur)->GetBufferSize()
	};
	horizontalBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	CreatePSO(SPSOType::HorizontalBlur, horizontalBlurPSO);

	D3D12_COMPUTE_PIPELINE_STATE_DESC verticalBlurPSO = {};
	verticalBlurPSO.pRootSignature = BlurRootSignature.Get();
	verticalBlurPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSVerticalBlur)->GetBufferPointer()),
		GetShader(SShaderTypes::CSVerticalBlur)->GetBufferSize()
	};
	verticalBlurPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	CreatePSO(SPSOType::VerticalBlur, verticalBlurPSO);
}
D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetReflectedPSODesc()
{
	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	reflectionsDSS.DepthEnable = true;
	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSS.StencilEnable = true;
	reflectionsDSS.StencilReadMask = 0xff;
	reflectionsDSS.StencilWriteMask = 0xff;

	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionsPsoDesc = GetOpaquePSODesc();
	reflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	reflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;
	return reflectionsPsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetMirrorPSODesc()
{
	CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = true;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = true;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;

	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	mirrorDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC mirrorPsoDesc = GetOpaquePSODesc();
	mirrorPsoDesc.BlendState = mirrorBlendState;
	mirrorPsoDesc.DepthStencilState = mirrorDSS;
	return mirrorPsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetDebugPSODesc()
{
	// wireframe debug
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = GetOpaquePSODesc();
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	return opaqueWireframePsoDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetCompositePSODesc()
{
	auto compositePSO = GetOpaquePSODesc();
	compositePSO.pRootSignature = PostProcessRootSignature.Get();
	compositePSO.DepthStencilState.DepthEnable = false;
	compositePSO.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	compositePSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	compositePSO.VS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::VSComposite)->GetBufferPointer()),
		GetShader(SShaderTypes::VSComposite)->GetBufferSize()
	};

	compositePSO.PS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::PSComposite)->GetBufferPointer()),
		GetShader(SShaderTypes::PSComposite)->GetBufferSize()
	};

	return compositePSO;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetSkyPSODesc()
{
	auto skyPSO = GetOpaquePSODesc();
	skyPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	skyPSO.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPSO.VS = GetShaderByteCode(SShaderTypes::VSSky);
	skyPSO.PS = GetShaderByteCode(SShaderTypes::PSSky);
	return skyPSO;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC OGraphicsPipelineManager::GetSobelPSODesc()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC sobelPSO = {};
	sobelPSO.pRootSignature = PostProcessRootSignature.Get();
	sobelPSO.CS = {
		reinterpret_cast<BYTE*>(GetShader(SShaderTypes::CSSobelFilter)->GetBufferPointer()),
		GetShader(SShaderTypes::CSSobelFilter)->GetBufferSize()
	};
	sobelPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	return sobelPSO;
}
