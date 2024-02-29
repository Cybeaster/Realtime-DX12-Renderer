//
// Created by Cybea on 27/02/2024.
//

#include "Compiler.h"

#include "../../Utils/DirectXUtils.h"
#include "Engine/Shader/Shader.h"
#include "GraphicsPipeline/GraphicsPipeline.h"
#include "Logger.h"

#include <ranges>
void OShaderCompiler::Init()
{
	THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
	THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)));
	THROW_IF_FAILED(Utils->CreateDefaultIncludeHandler(&IncludeHandler));
}

unique_ptr<OShader> OShaderCompiler::CompilerShader(const SShaderDefinition& Definition, wstring ShaderPath, SPipelineInfo& OutPipelineInfo)
{
	CompilationArgs = {
		L"-E",
		UTF8ToWString(Definition.ShaderEntry).c_str(),
		L"-T",
		UTF8ToWString(Definition.TargetProfile).c_str(),
		DXC_ARG_PACK_MATRIX_ROW_MAJOR,
		DXC_ARG_WARNINGS_ARE_ERRORS,
		DXC_ARG_ALL_RESOURCES_BOUND,
	};

	if constexpr (DEBUG)
	{
		CompilationArgs.push_back(DXC_ARG_DEBUG);
		CompilationArgs.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
	}
	else
	{
		CompilationArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
	}

	ComPtr<IDxcBlobEncoding> sourceBlob;
	THROW_IF_FAILED(Utils->LoadFile(ShaderPath.c_str(), nullptr, &sourceBlob));
	DxcBuffer sourceBuffer{
		.Ptr = sourceBlob->GetBufferPointer(),
		.Size = sourceBlob->GetBufferSize(),
		.Encoding = 0
	};
	ComPtr<IDxcResult> compiledShaderBuffer{};
	const HRESULT hr = Compiler->Compile(&sourceBuffer,
	                                     CompilationArgs.data(),
	                                     static_cast<uint32_t>(CompilationArgs.size()),
	                                     IncludeHandler.Get(),
	                                     IID_PPV_ARGS(&compiledShaderBuffer));

	if (FAILED(hr))
	{
		WIN_LOG(Engine, Error, "Failed to compile shader: {}", ShaderPath);
	}

	ComPtr<IDxcBlobUtf8> errors{};
	THROW_IF_FAILED(compiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
	if (errors && errors->GetStringLength() > 0)
	{
		WIN_LOG(Engine, Error, "Shader compilation error: {}", errors->GetStringPointer());
	}

	ComPtr<IDxcBlob> reflectionBlob{};
	THROW_IF_FAILED(compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&reflectionBlob), nullptr));

	const DxcBuffer reflectionBuffer{
		.Ptr = reflectionBlob->GetBufferPointer(),
		.Size = reflectionBlob->GetBufferSize(),
		.Encoding = 0
	};

	ComPtr<ID3D12ShaderReflection> shaderReflection{};
	Utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
	D3D12_SHADER_DESC shaderDesc{};
	shaderReflection->GetDesc(&shaderDesc);
	D3D12_INPUT_LAYOUT_DESC layoutDesc{};
	if (Definition.ShaderType == EShaderLevel::VertexShader)
	{
		vector<string> inputElementSemanticNames;
		vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

		inputElementSemanticNames.reserve(shaderDesc.InputParameters);
		inputElementDescs.reserve(shaderDesc.InputParameters);

		for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
		{
			D3D12_SIGNATURE_PARAMETER_DESC parameterDesc{};
			shaderReflection->GetInputParameterDesc(parameterIndex, &parameterDesc);

			inputElementSemanticNames.emplace_back(parameterDesc.SemanticName);
			inputElementDescs.push_back(D3D12_INPUT_ELEMENT_DESC{
			    .SemanticName = inputElementSemanticNames.back().c_str(),
			    .SemanticIndex = parameterDesc.SemanticIndex,
			    .Format = Utils::MaskToFormat(parameterDesc.Mask),
			    .InputSlot = 0u,
			    .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
			    .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			    .InstanceDataStepRate = 0u,

			});
		}
		layoutDesc = {
			.NumElements = static_cast<uint32_t>(inputElementDescs.size()),
			.pInputElementDescs = inputElementDescs.data()
		};
	}

	for (const uint32_t i : std::views::iota(0u, shaderDesc.BoundResources))
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		shaderReflection->GetResourceBindingDesc(i, &bindDesc);

		if (bindDesc.Type == D3D_SIT_CBUFFER)
		{
			OutPipelineInfo.RootParamIndexMap[UTF8ToWString(bindDesc.Name)] = bindDesc.BindPoint;
			ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
			shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

			const D3D12_ROOT_PARAMETER1 rootParameter{
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
				.Descriptor{
				    .ShaderRegister = bindDesc.BindPoint,
				    .RegisterSpace = bindDesc.Space,
				    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
				},
			};
			OutPipelineInfo.RootParameters.push_back(rootParameter);
		}

		if (bindDesc.Type == D3D_SIT_TEXTURE)
		{
			OutPipelineInfo.RootParamIndexMap[UTF8ToWString(bindDesc.Name)] = bindDesc.BindPoint;
			D3D12_DESCRIPTOR_RANGE1 texturesDescriptorRange = {
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				.NumDescriptors = 9, // gTextureMaps[7], gDisplacementMap, gCubeMap
				.BaseShaderRegister = 0, // Starting at t0
				.RegisterSpace = 0, // Default space
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
			};

			OutPipelineInfo.DescriptorRanges.push_back(texturesDescriptorRange);
			const D3D12_ROOT_PARAMETER1 rootParameter{
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable{
				    .NumDescriptorRanges = 1u,
				    .pDescriptorRanges = &OutPipelineInfo.DescriptorRanges.back(),
				},
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
			};
			OutPipelineInfo.RootParameters.push_back(rootParameter);

			// For Structured Buffers in space1, define descriptor ranges and root parameters individually
			std::array<D3D12_DESCRIPTOR_RANGE1, 2> sbDescriptorRanges = { {
				{
				    // gInstanceData
				    .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				    .NumDescriptors = 1,
				    .BaseShaderRegister = 0, // t0 in space1
				    .RegisterSpace = 1,
				    .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
				},
				{
				    // gMaterialData
				    .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				    .NumDescriptors = 1,
				    .BaseShaderRegister = 1, // t1 in space1
				    .RegisterSpace = 1,
				    .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
				},
			} };

			for (const auto& range : sbDescriptorRanges)
			{
				D3D12_ROOT_PARAMETER1 sbTableParam = {
					.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
					.DescriptorTable = {
					    .NumDescriptorRanges = 1,
					    .pDescriptorRanges = &range,
					},
					.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
				};
				OutPipelineInfo.RootParameters.push_back(sbTableParam);
			}
		}
	}
	ComPtr<IDxcBlob> compiledShaderBlob;
	THROW_IF_FAILED(compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShaderBlob), nullptr));

	auto shader = make_unique<OShader>();
	auto samplers = Utils::GetStaticSamplers();
	OutPipelineInfo.RootSignatureDesc = {
		.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
		.Desc_1_1 = {
		    .NumParameters = static_cast<uint32_t>(OutPipelineInfo.RootParameters.size()),
		    .pParameters = OutPipelineInfo.RootParameters.data(),
		    .NumStaticSamplers = samplers.size(),
		    .pStaticSamplers = samplers.data(),
		    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
		},
	};
	shader->Init(Definition, compiledShaderBlob, layoutDesc);
	return std::move(shader);
}
