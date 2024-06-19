
#include "Compiler.h"

#include "Application.h"
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

bool OShaderCompiler::CompileShaders(vector<SPipelineStage>& OutPipelines, SShaderPipelineDesc& OutShadersPipeline, vector<unique_ptr<OShader>>& OutResult)
{
	vector<unique_ptr<OShader>> shaders;
	for (auto& shader : OutPipelines)
	{
		LOG(Engine, Log, "Compiling shader: {}", shader.ShaderPath);

		auto temp = CompileShader(shader.ShaderDefinition, shader.Defines, shader.ShaderPath, OutShadersPipeline);
		if (temp == nullptr)
		{
			return false;
		}

		shader.Shader = temp.get();
		OutResult.push_back(std::move(temp));
	}
	auto rootSig = BuildRootSignature(OutShadersPipeline.BuildParameterArray(), Utils::GetStaticSamplers(), OutShadersPipeline.RootSignatureParams.RootSignatureDesc);
	OutShadersPipeline.RootSignatureParams.RootSignature = rootSig;

	return true;
}

void OShaderCompiler::SetCompilationArgs(const SShaderDefinition& Definition, const vector<SShaderMacro>& Macros)
{
	CompilationArgs = {
		L"-E",
		Definition.ShaderEntry,
		L"-T",
		Definition.TargetProfile,
		DXC_ARG_WARNINGS_ARE_ERRORS,
		DXC_ARG_ALL_RESOURCES_BOUND,
	};

	for (auto& folder : OApplication::Get()->GetShaderFolders())
	{
		CompilationArgs.push_back(L"-I");
		CompilationArgs.push_back(folder.c_str());
	}

	for (auto& macro : Macros)
	{
		CompilationArgs.push_back(L"-D");
		CompilationArgs.push_back(UTF8ToWString(macro.Name));
		CompilationArgs.push_back(UTF8ToWString(macro.Definition));
	}

	if constexpr (DEBUG)
	{
		CompilationArgs.push_back(DXC_ARG_DEBUG);
		CompilationArgs.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
	}
	else
	{
		CompilationArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
	}
}

void OShaderCompiler::ResolveBoundResources(const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_DESC& ShaderDescription, SShaderPipelineDesc& OutPipelineInfo, EShaderLevel ShaderType)
{
	OutPipelineInfo.RootSignatureParams.DescriptorRanges.reserve(50); // TODO infer from shader
	uint32_t counter = OutPipelineInfo.GetRootParamIndexMap().size();
	for (const uint32_t i : std::views::iota(0u, ShaderDescription.BoundResources))
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		Reflection->GetResourceBindingDesc(i, &bindDesc);

		if (bindDesc.Type == D3D_SIT_SAMPLER)
		{
			continue;
		}

		if (!OutPipelineInfo.TryAddRootParameterName(UTF8ToWString(bindDesc.Name)))
		{
			continue;
		}

		OutPipelineInfo.GetRootParamIndexMap()[UTF8ToWString(bindDesc.Name)] = counter;
		counter += 1;
		switch (bindDesc.Type)
		{
		case D3D_SIT_CBUFFER:
			ResolveConstantBuffers(i, Reflection, bindDesc, OutPipelineInfo);
			break;
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_TEXTURE:
			ResolveTextures(bindDesc, OutPipelineInfo, ShaderType);
			break;
		case D3D_SIT_STRUCTURED:
			ResolveStructuredBuffer(bindDesc, OutPipelineInfo, ShaderType);
			break;
		// Handle other types as needed, for example, samplers
		case D3D_SIT_RTACCELERATIONSTRUCTURE:
			ResolveStructuredBuffer(bindDesc, OutPipelineInfo, ShaderType);
			break;
		}
	}
}

void OShaderCompiler::ResolveConstantBuffers(const int32_t ResourceIdx, const ComPtr<ID3D12ShaderReflection>& Reflection, const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SShaderPipelineDesc& OutPipelineInfo)
{
	auto name = UTF8ToWString(BindDesc.Name);
	ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = Reflection->GetConstantBufferByIndex(ResourceIdx);
	D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
	shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

	const D3D12_ROOT_PARAMETER1 rootParameter{
		.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
		.Descriptor = {
		    .ShaderRegister = BindDesc.BindPoint,
		    .RegisterSpace = BindDesc.Space,
		    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
		}
	};
	OutPipelineInfo.AddRootParameter(rootParameter, name);
}

void OShaderCompiler::ResolveStructuredBuffer(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SShaderPipelineDesc& OutPipelineInfo, EShaderLevel ShaderType)
{
	auto name = UTF8ToWString(BindDesc.Name);
	CHECK(BindDesc.BindCount > 0);

	D3D12_ROOT_PARAMETER1 rootParameter = {
		.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
		.Descriptor = {
		    .ShaderRegister = BindDesc.BindPoint,
		    .RegisterSpace = BindDesc.Space,
		    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
		},
		.ShaderVisibility = ShaderTypeToVisibility(ShaderType),
	};

	OutPipelineInfo.AddRootParameter(rootParameter, name);
}

void OShaderCompiler::ResolveTextures(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc, SShaderPipelineDesc& OutPipelineInfo, EShaderLevel ShaderType)
{
	auto name = UTF8ToWString(BindDesc.Name);
	CHECK(BindDesc.BindCount > 0);
	D3D12_DESCRIPTOR_RANGE1 descriptorRange = {
		.RangeType = GetRangeType(BindDesc),
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
		.ShaderVisibility = ShaderTypeToVisibility(ShaderType),
	};

	OutPipelineInfo.AddRootParameter(rootParameter, name);
}
D3D12_DESCRIPTOR_RANGE_TYPE OShaderCompiler::GetRangeType(const D3D12_SHADER_INPUT_BIND_DESC& BindDesc)
{
	switch (BindDesc.Type)
	{
	case D3D_SIT_CBUFFER:
		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case D3D_SIT_TBUFFER:
	case D3D_SIT_TEXTURE:
	case D3D_SIT_STRUCTURED:
	case D3D_SIT_BYTEADDRESS:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case D3D_SIT_SAMPLER:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	case D3D_SIT_UAV_RWTYPED:
	case D3D_SIT_UAV_RWSTRUCTURED:
	case D3D_SIT_UAV_RWBYTEADDRESS:
	case D3D_SIT_UAV_APPEND_STRUCTURED:
	case D3D_SIT_UAV_CONSUME_STRUCTURED:
	case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
	case D3D_SIT_UAV_FEEDBACKTEXTURE:
		return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case D3D_SIT_RTACCELERATIONSTRUCTURE:
		return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	default:
		LOG(Render, Error, "Unexpected shader input bind type encountered.");
		break;
	}
	return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
}

bool OShaderCompiler::CreateDxcBuffer(const wstring& ShaderPath, ComPtr<IDxcResult>& OutCompiledShaderBuffer, DxcBuffer& OutReflectionBuffe)
{
	ComPtr<IDxcBlobEncoding> sourceBlob;
	THROW_IF_FAILED(Utils->LoadFile(ShaderPath.c_str(), nullptr, &sourceBlob));
	DxcBuffer sourceBuffer{
		.Ptr = sourceBlob->GetBufferPointer(),
		.Size = sourceBlob->GetBufferSize(),
		.Encoding = 0
	};

	vector<LPCWSTR> args;
	for (auto& arg : CompilationArgs)
	{
		args.push_back(arg.c_str());
	}

	const HRESULT hr = Compiler->Compile(&sourceBuffer,
	                                     args.data(),
	                                     static_cast<uint32_t>(args.size()),
	                                     IncludeHandler.Get(),
	                                     IID_PPV_ARGS(&OutCompiledShaderBuffer));

	if (FAILED(hr))
	{
		LOG(Engine, Error, "Failed to compile shader: {}", ShaderPath);
	}

	ComPtr<IDxcBlobUtf8> errors{};
	OutCompiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	if (errors && errors->GetStringLength() > 0)
	{
		LOG(Engine, Error, "Shader compilation error: {}", TEXT(errors->GetStringPointer()));
		return false;
	}

	ComPtr<IDxcBlob> reflectionBlob{};
	OutCompiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&reflectionBlob), nullptr);

	OutReflectionBuffe = {
		.Ptr = reflectionBlob->GetBufferPointer(),
		.Size = reflectionBlob->GetBufferSize(),
		.Encoding = 0
	};
	return true;
}

void OShaderCompiler::GetInputLayoutDesc(const ComPtr<ID3D12ShaderReflection>& Reflection, SShaderPipelineDesc& OutPipelineInfo)
{
	D3D12_SHADER_DESC shaderDesc{};
	Reflection->GetDesc(&shaderDesc);
	OutPipelineInfo.InputElementSemanticNames.reserve(shaderDesc.InputParameters);
	OutPipelineInfo.InputElementDescs.reserve(shaderDesc.InputParameters);

	for (const uint32_t parameterIndex : std::views::iota(0u, shaderDesc.InputParameters))
	{
		D3D12_SIGNATURE_PARAMETER_DESC parameterDesc{};
		Reflection->GetInputParameterDesc(parameterIndex, &parameterDesc);

		OutPipelineInfo.InputElementSemanticNames.emplace_back(parameterDesc.SemanticName);
		OutPipelineInfo.InputElementDescs.push_back(D3D12_INPUT_ELEMENT_DESC{
		    .SemanticName = OutPipelineInfo.InputElementSemanticNames.back().c_str(),
		    .SemanticIndex = parameterDesc.SemanticIndex,
		    .Format = Utils::MaskToFormat(parameterDesc.Mask),
		    .InputSlot = 0u,
		    .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
		    .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		    .InstanceDataStepRate = 0u,

		});
	}
}

bool OShaderCompiler::BuildShaderReflection(DxcBuffer Buffer, ComPtr<ID3D12ShaderReflection>& OutReflection, D3D12_SHADER_DESC& OutShaderDesc)
{
	auto res = Utils->CreateReflection(&Buffer, IID_PPV_ARGS(&OutReflection));
	if (SUCCEEDED(res))
	{
		OutReflection->GetDesc(&OutShaderDesc);
	}
	else
	{
		LOG(Engine, Error, "Failed to create shader reflection. {}", TEXT(res));
		return false;
	}

	return true;
}

unique_ptr<OShader> OShaderCompiler::CompileShader(const SShaderDefinition& Definition, const vector<SShaderMacro>& Macros, const wstring& ShaderPath, SShaderPipelineDesc& OutPipelineInfo)
{
	SetCompilationArgs(Definition, Macros);
	ComPtr<IDxcResult> compiledShaderBuffer;
	DxcBuffer reflectionBuffe;
	if (!CreateDxcBuffer(ShaderPath, compiledShaderBuffer, reflectionBuffe))
	{
		return nullptr;
	}

	ComPtr<ID3D12ShaderReflection> shaderReflection{};
	D3D12_SHADER_DESC shaderDesc;
	if (!BuildShaderReflection(reflectionBuffe, shaderReflection, shaderDesc))
	{
		return nullptr;
	}

	ResolveBoundResources(shaderReflection, shaderDesc, OutPipelineInfo, Definition.ShaderType);
	if (Definition.ShaderType == EShaderLevel::VertexShader)
	{
		GetInputLayoutDesc(shaderReflection, OutPipelineInfo);
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
	const auto numSamples = static_cast<uint32_t>(StaticSamplers.size());

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
	Utils::BuildRootSignature(OEngine::Get()->GetDevice().lock(), rootSignature, OutDescription);
	return rootSignature;
}
