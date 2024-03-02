
#include "Compiler.h"

#include "../../Utils/DirectXUtils.h"
#include "Engine/Engine.h"
#include "Engine/Shader/Shader.h"
#include "Logger.h"

#include <ranges>

void OShaderCompiler::Init()
{
	THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
	THROW_IF_FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)));
	THROW_IF_FAILED(Utils->CreateDefaultIncludeHandler(&IncludeHandler));
}

vector<unique_ptr<OShader>> OShaderCompiler::CompileShaders(const vector<SPipelineStage>& OutPipelines, SPipelineInfo& OutShadersPipeline)
{
	vector<unique_ptr<OShader>> shaders;
	for (auto shader : OutPipelines)
	{
		auto temp = CompileShader(shader.ShaderDefinition, shader.ShaderPath, OutShadersPipeline);
		shader.Shader = temp.get();
		shaders.push_back(std::move(temp));
	}
	BuildRootSignature(OutShadersPipeline.RootSignatureParams.RootParameters, Utils::GetStaticSamplers(), OutShadersPipeline.RootSignatureParams.RootSignatureDesc);
	return shaders;
}

void OShaderCompiler::SetCompilationArgs(const SShaderDefinition& Definition)
{
	Entry = Definition.ShaderEntry;
	TargetProfile = Definition.TargetProfile;
	CompilationArgs = {
		L"-E",
		Entry.c_str(),
		L"-T",
		TargetProfile.c_str(),
		DXC_ARG_PACK_MATRIX_ROW_MAJOR,
		DXC_ARG_WARNINGS_ARE_ERRORS,
		DXC_ARG_ALL_RESOURCES_BOUND,
		L"-I",
		L"Shaders/",
	};

	if constexpr (_DEBUG)
	{
		CompilationArgs.push_back(DXC_ARG_DEBUG);
		CompilationArgs.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
	}
	else
	{
		CompilationArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
	}
}

void OShaderCompiler::ResolveBoundResources(const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_DESC& ShaderDescription, SPipelineInfo& OutPipelineInfo)
{
	for (const uint32_t i : std::views::iota(0u, ShaderDescription.BoundResources))
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		Reflection->GetResourceBindingDesc(i, &bindDesc);
		if (!OutPipelineInfo.TryAddRootParamterName(UTF8ToWString(bindDesc.Name))) //TODO too long names
		{
			continue;
		}

		switch (bindDesc.Type)
		{
		case D3D_SIT_CBUFFER:
			ResolveConstantBuffers(i, Reflection, bindDesc, OutPipelineInfo);
			break;
		case D3D_SIT_TEXTURE:
		case D3D_SIT_STRUCTURED:
			ResolveTexturesAndStructuredBuffers(bindDesc, OutPipelineInfo);
			break;
			// Handle other types as needed, for example, samplers
		}
	}
}

void OShaderCompiler::ResolveConstantBuffers(const int32_t ResourceIdx, const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SPipelineInfo& OutPipelineInfo)
{
	OutPipelineInfo.RootParamIndexMap[UTF8ToWString(BindDesc.Name)] = BindDesc.BindPoint;
	ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = Reflection->GetConstantBufferByIndex(ResourceIdx);
	D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
	shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

	const D3D12_ROOT_PARAMETER1 rootParameter{
		.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
		.Descriptor{
		    .ShaderRegister = BindDesc.BindPoint,
		    .RegisterSpace = BindDesc.Space,
		    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
		},
	};
	OutPipelineInfo.AddRootParameter(rootParameter);
}

void OShaderCompiler::ResolveTexturesAndStructuredBuffers(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SPipelineInfo& OutPipelineInfo)
{
	D3D12_DESCRIPTOR_RANGE1 descriptorRange = {
		.RangeType = (BindDesc.Type == D3D_SIT_CBUFFER) ? D3D12_DESCRIPTOR_RANGE_TYPE_CBV : D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = BindDesc.BindCount, // Special handling for structured buffers in space1
		.BaseShaderRegister = BindDesc.BindPoint,
		.RegisterSpace = BindDesc.Space,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	OutPipelineInfo.RootSignatureParams.DescriptorRanges.push_back(descriptorRange);

	D3D12_ROOT_PARAMETER1 rootParameter = {
		.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
		.DescriptorTable = {
		    .NumDescriptorRanges = 1,
		    .pDescriptorRanges = &OutPipelineInfo.RootSignatureParams.DescriptorRanges.back(),
		},
		.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL, // Or specific visibility as needed
	};

	OutPipelineInfo.RootSignatureParams.RootParameters.push_back(rootParameter);
}

std::tuple<DxcBuffer, ComPtr<IDxcResult>> OShaderCompiler::CreateDxcBuffer(const wstring& ShaderPath)
{
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
		WIN_LOG(Engine, Error, "Shader compilation error: {}", TEXT(errors->GetStringPointer()));
		return {};
	}

	ComPtr<IDxcBlob> reflectionBlob{};
	THROW_IF_FAILED(compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&reflectionBlob), nullptr));

	const DxcBuffer reflectionBuffer{
		.Ptr = reflectionBlob->GetBufferPointer(),
		.Size = reflectionBlob->GetBufferSize(),
		.Encoding = 0
	};
	return { reflectionBuffer, compiledShaderBuffer };
}

D3D12_INPUT_LAYOUT_DESC OShaderCompiler::GetInputLayoutDesc(const ComPtr<ID3D12ShaderReflection>& Reflection)
{
	D3D12_INPUT_LAYOUT_DESC layoutDesc{};
	vector<string> inputElementSemanticNames;
	vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
	D3D12_SHADER_DESC shaderDesc{};

	Reflection->GetDesc(&shaderDesc);
	inputElementSemanticNames.reserve(shaderDesc.InputParameters);
	inputElementDescs.reserve(shaderDesc.InputParameters);

	for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
	{
		D3D12_SIGNATURE_PARAMETER_DESC parameterDesc{};
		Reflection->GetInputParameterDesc(parameterIndex, &parameterDesc);

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
		.pInputElementDescs = inputElementDescs.data(),
		.NumElements = static_cast<uint32_t>(inputElementDescs.size())
	};

	return layoutDesc;
}

D3D12_SHADER_DESC OShaderCompiler::BuildReflection(DxcBuffer Buffer, ComPtr<ID3D12ShaderReflection>& OutReflection)
{
	D3D12_SHADER_DESC shaderDesc{};
	Utils->CreateReflection(&Buffer, IID_PPV_ARGS(&OutReflection));
	OutReflection->GetDesc(&shaderDesc);
	return shaderDesc;
}

unique_ptr<OShader> OShaderCompiler::CompileShader(const SShaderDefinition& Definition, wstring ShaderPath, SPipelineInfo& OutPipelineInfo)
{
	SetCompilationArgs(Definition);
	auto [buffer, compiledShaderBuffer] = CreateDxcBuffer(ShaderPath);

	ComPtr<ID3D12ShaderReflection> shaderReflection{};
	const D3D12_SHADER_DESC shaderDesc = BuildReflection(buffer, shaderReflection);

	ResolveBoundResources(shaderReflection, shaderDesc, OutPipelineInfo);
	if (Definition.ShaderType == EShaderLevel::VertexShader)
	{
		OutPipelineInfo.InputLayoutDesc = GetInputLayoutDesc(shaderReflection);
	}
	ComPtr<IDxcBlob> compiledShaderBlob;
	THROW_IF_FAILED(compiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&compiledShaderBlob), nullptr));

	auto shader = make_unique<OShader>();
	shader->Init(Definition, compiledShaderBlob);
	return std::move(shader);
}

ComPtr<ID3D12RootSignature> OShaderCompiler::BuildRootSignature(vector<D3D12_ROOT_PARAMETER1>& RootParameter, const vector<CD3DX12_STATIC_SAMPLER_DESC>& StaticSamplers, D3D12_VERSIONED_ROOT_SIGNATURE_DESC& OutDescription)
{
	ComPtr<ID3D12RootSignature> rootSignature;
	const uint32_t numSamples = static_cast<uint32_t>(StaticSamplers.size());
	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc = {
		.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
		.Desc_1_1 = {
		    .NumParameters = static_cast<uint32_t>(RootParameter.size()),
		    .pParameters = RootParameter.data(),
		    .NumStaticSamplers = numSamples,
		    .pStaticSamplers = StaticSamplers.data(),
		    .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
		},
	};
	OutDescription = desc;
	Utils::BuildRootSignature(OEngine::Get()->GetDevice().Get(), rootSignature, OutDescription);
	return rootSignature;
}