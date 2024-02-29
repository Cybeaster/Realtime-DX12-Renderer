#pragma once
#include "DXHelper.h"
#include "Engine/Shader/Shader.h"
#include "Types.h"

#include <d3d12shader.h> // Contains functions and structures useful in accessing shader information.
#include <dxcapi.h>

struct SPipelineInfo;
class OShaderCompiler
{
public:
	void Init();
	unique_ptr<OShader> CompilerShader(const SShaderDefinition& Definition, wstring ShaderPath, SPipelineInfo& OutPipelineInfo);

private:
	ComPtr<IDxcCompiler3> Compiler;
	ComPtr<IDxcUtils> Utils;
	ComPtr<IDxcIncludeHandler> IncludeHandler;
	vector<LPCWSTR> CompilationArgs;
};
