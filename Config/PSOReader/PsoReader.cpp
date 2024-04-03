#include "PsoReader.h"

#include <ranges>

vector<unique_ptr<SPSODescriptionBase>> OPSOReader::LoadPSOs() const
{
	vector<unique_ptr<SPSODescriptionBase>> PSOs;
	for (auto val : GetRootChild("PipelineStateObjects") | std::views::values)
	{
		auto type = val.get<string>("Type");
		if (type == "Graphics")
		{
			PSOs.push_back(LoadGraphicsPSO(val));
		}
		else if (type == "Compute")
		{
			PSOs.push_back(LoadComputePSO(val));
		}
	}
	return PSOs;
}

unique_ptr<SPSOGraphicsDescription> OPSOReader::LoadGraphicsPSO(const boost::property_tree::ptree& Node) const
{
	auto PSODesc = make_unique<SPSOGraphicsDescription>();
	PSODesc->Type = EPSOType::Graphics;
	auto& desc = PSODesc->PSODesc;
	PSODesc->Name = Node.get<string>("Name");
	PSODesc->RootSignatureName = GetAttribute(Node, "RootSignature");
	PSODesc->ShaderPipeline = GetShaderArray(Node.get_child("ShaderPipeline"));
	desc.Flags = GetFlags(Node);
	desc.SampleMask = GetOptionalOr(Node, "SampleMask", UINT_MAX);
	desc.PrimitiveTopologyType = GetTopologyType(Node);
	desc.NumRenderTargets = GetOptionalOr(Node, "NumRenderTargets", 1);
	SetRenderTargetFormats(desc.RTVFormats, Node);
	desc.DSVFormat = GetFormat(Node);
	desc.SampleDesc = GetSampleDescription(Node);
	desc.BlendState = GetBlendDesc(Node);
	desc.RasterizerState = GetRasterizerDesc(Node);
	desc.DepthStencilState = GetDepthStencilDesc(Node);

	if (desc.BlendState.RenderTarget[0].LogicOpEnable == 1 && desc.RTVFormats[0] == DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		LOG(Config, Error, "DXGI_FORMAT_R8G8B8A8_UNORM does not support logic op!");
	}

	return PSODesc;
}

unique_ptr<SPSOComputeDescription> OPSOReader::LoadComputePSO(const boost::property_tree::ptree& Node) const
{
	auto PSODesc = make_unique<SPSOComputeDescription>();
	PSODesc->Type = EPSOType::Compute;
	auto& desc = PSODesc->PSODesc;
	PSODesc->Name = GetAttribute(Node, "Name");
	PSODesc->RootSignatureName = GetAttribute(Node, "RootSignature");
	PSODesc->ShaderPipeline = GetShaderArray(Node.get_child("ShaderPipeline"));
	ENSURE(PSODesc->ShaderPipeline.ComputeShaderName.empty() == false);
	desc.Flags = GetFlags(Node);
	return PSODesc;
}

DXGI_SAMPLE_DESC OPSOReader::GetSampleDescription(const boost::property_tree::ptree& Node)
{
	if (auto optional = Node.get_child_optional("SampleDesc"))
	{
		DXGI_SAMPLE_DESC desc;
		desc.Count = optional.value().get<UINT>("Count");
		desc.Quality = optional.value().get<UINT>("Quality");
		return desc;
	}
	return { 1, 0 };
}

CD3DX12_BLEND_DESC OPSOReader::GetBlendDesc(const boost::property_tree::ptree& Node)
{
	if (const auto optional = Node.get_child_optional("BlendState"))
	{
		CD3DX12_BLEND_DESC desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.AlphaToCoverageEnable = GetOptionalOr(optional, "AlphaToCoverageEnable", false);
		desc.IndependentBlendEnable = GetOptionalOr(optional, "IndependentBlendEnable", false);

		for (uint32_t counter = 0; counter < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; counter++)
		{
			if (const auto renderTargetOptional = optional->get_child_optional("RenderTarget"))
			{
				for (const auto& val : *renderTargetOptional | std::views::values)
				{
					auto& target = desc.RenderTarget[counter];
					target.BlendEnable = GetOptionalOr(val, "BlendEnable", false);
					target.LogicOpEnable = GetOptionalOr(val, "LogicOpEnable", false);
					target.SrcBlend = GetBlend(GetOptionalOr(val, "SrcBlend", "One"));
					target.DestBlend = GetBlend(GetOptionalOr(val, "DestBlend", "Zero"));
					target.BlendOp = GetBlendOp(GetOptionalOr(val, "BlendOp", "Add"));
					target.SrcBlendAlpha = GetBlend(GetOptionalOr(val, "SrcBlendAlpha", "One"));
					target.DestBlendAlpha = GetBlend(GetOptionalOr(val, "DestBlendAlpha", "Zero"));
					target.BlendOpAlpha = GetBlendOp(GetOptionalOr(val, "BlendOpAlpha", "Add"));
					target.LogicOp = GetLogicOp(GetOptionalOr(val, "LogicOp", "NoOp"));
					target.RenderTargetWriteMask = GetColorWriteEnable(GetAttribute(val, "RenderTargetWriteMask"));

					counter++;
				}
			}
		}

		return desc;
	}
	return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
}

D3D12_LOGIC_OP OPSOReader::GetLogicOp(const string& LogicOpString)
{
	if (LogicOpString == "Clear")
	{
		return D3D12_LOGIC_OP_CLEAR;
	}
	if (LogicOpString == "Set")
	{
		return D3D12_LOGIC_OP_SET;
	}
	if (LogicOpString == "Copy")
	{
		return D3D12_LOGIC_OP_COPY;
	}
	if (LogicOpString == "CopyInverted")
	{
		return D3D12_LOGIC_OP_COPY_INVERTED;
	}
	if (LogicOpString == "NoOp")
	{
		return D3D12_LOGIC_OP_NOOP;
	}
	if (LogicOpString == "Invert")
	{
		return D3D12_LOGIC_OP_INVERT;
	}
	if (LogicOpString == "And")
	{
		return D3D12_LOGIC_OP_AND;
	}
	if (LogicOpString == "Nand")
	{
		return D3D12_LOGIC_OP_NAND;
	}
	if (LogicOpString == "Or")
	{
		return D3D12_LOGIC_OP_OR;
	}
	WIN_LOG(Config, Error, "Unknown logic op: {}", TEXT(LogicOpString));
	return D3D12_LOGIC_OP_CLEAR;
}

D3D12_BLEND_OP OPSOReader::GetBlendOp(const string& BlendOpString)
{
	if (BlendOpString == "Add")
	{
		return D3D12_BLEND_OP_ADD;
	}
	if (BlendOpString == "Subtract")
	{
		return D3D12_BLEND_OP_SUBTRACT;
	}
	if (BlendOpString == "RevSubtract")
	{
		return D3D12_BLEND_OP_REV_SUBTRACT;
	}
	if (BlendOpString == "Min")
	{
		return D3D12_BLEND_OP_MIN;
	}
	if (BlendOpString == "Max")
	{
		return D3D12_BLEND_OP_MAX;
	}
	WIN_LOG(Config, Error, "Unknown logic op: {}", TEXT(BlendOpString));
	return D3D12_BLEND_OP_ADD;
}

D3D12_BLEND OPSOReader::GetBlend(const string& BlendString)
{
	if (BlendString == "Zero")
	{
		return D3D12_BLEND_ZERO;
	}
	if (BlendString == "One")
	{
		return D3D12_BLEND_ONE;
	}
	if (BlendString == "SrcColor")
	{
		return D3D12_BLEND_SRC_COLOR;
	}
	if (BlendString == "InvSrcColor")
	{
		return D3D12_BLEND_INV_SRC_COLOR;
	}
	if (BlendString == "SrcAlpha")
	{
		return D3D12_BLEND_SRC_ALPHA;
	}
	if (BlendString == "InvSrcAlpha")
	{
		return D3D12_BLEND_INV_SRC_ALPHA;
	}
	if (BlendString == "DestAlpha")
	{
		return D3D12_BLEND_DEST_ALPHA;
	}
	if (BlendString == "InvDestAlpha")
	{
		return D3D12_BLEND_INV_DEST_ALPHA;
	}
	if (BlendString == "DestColor")
	{
		return D3D12_BLEND_DEST_COLOR;
	}
	if (BlendString == "InvDestColor")
	{
		return D3D12_BLEND_INV_DEST_COLOR;
	}
	if (BlendString == "SrcAlphaSat")
	{
		return D3D12_BLEND_SRC_ALPHA_SAT;
	}
	WIN_LOG(Config, Error, "Unknown blend op: {}", TEXT(BlendString));
	return D3D12_BLEND_ZERO;
}

CD3DX12_RASTERIZER_DESC OPSOReader::GetRasterizerDesc(const boost::property_tree::ptree& Node)
{
	CD3DX12_RASTERIZER_DESC desc;
	if (auto optional = Node.get_child_optional("RasterizerState"))
	{
		const auto& value = optional.get();
		desc.FrontCounterClockwise = GetOptionalOr(value, "FrontCounterClockwise", false);
		desc.FillMode = GetFillMode(GetOptionalOr(value, "FillMode", "Solid"));
		desc.CullMode = GetCullMode(GetOptionalOr(value, "CullMode", "Back"));
		desc.DepthBias = GetOptionalOr(value, "DepthBias", 0);
		desc.DepthBiasClamp = GetOptionalOr(value, "DepthBiasClamp", 0.f);
		desc.SlopeScaledDepthBias = GetOptionalOr(value, "SlopeScaledDepthBias", 0.f);
		desc.DepthClipEnable = GetOptionalOr(value, "DepthClipEnable", true);
		desc.MultisampleEnable = GetOptionalOr(value, "MultisampleEnable", false);
		desc.AntialiasedLineEnable = GetOptionalOr(value, "AntialiasedLineEnable", false);
		desc.ForcedSampleCount = GetOptionalOr(value, "ForcedSampleCount", 0);
		desc.ConservativeRaster = GetConservativeRasterizationMode(GetOptionalOr(value, "ConservativeRaster", "Off"));
	}
	return desc;
}

D3D12_CULL_MODE OPSOReader::GetCullMode(const string& CullModeString)
{
	if (CullModeString == "None")
	{
		return D3D12_CULL_MODE_NONE;
	}
	if (CullModeString == "Front")
	{
		return D3D12_CULL_MODE_FRONT;
	}
	if (CullModeString == "Back")
	{
		return D3D12_CULL_MODE_BACK;
	}
	WIN_LOG(Config, Error, "Unknown cull mode: {}", TEXT(CullModeString));
	return D3D12_CULL_MODE_NONE;
}

D3D12_CONSERVATIVE_RASTERIZATION_MODE OPSOReader::GetConservativeRasterizationMode(const string& ConservativeRasterizationModeString)
{
	if (ConservativeRasterizationModeString == "Off")
	{
		return D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}
	if (ConservativeRasterizationModeString == "On")
	{
		return D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	}
	WIN_LOG(Config, Error, "Unknown logic op: {}", TEXT(ConservativeRasterizationModeString));
	return D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

D3D12_FILL_MODE OPSOReader::GetFillMode(const string& FillModeString)
{
	if (FillModeString == "Wireframe")
	{
		return D3D12_FILL_MODE_WIREFRAME;
	}
	if (FillModeString == "Solid")
	{
		return D3D12_FILL_MODE_SOLID;
	}
	WIN_LOG(Config, Error, "Unknown fill mode: {}", TEXT(FillModeString));
	return D3D12_FILL_MODE_SOLID;
}

CD3DX12_DEPTH_STENCIL_DESC OPSOReader::GetDepthStencilDesc(const boost::property_tree::ptree& Node)
{
	CD3DX12_DEPTH_STENCIL_DESC desc;
	if (auto optinal = Node.get_child_optional("DepthStencilState"))
	{
		desc.DepthEnable = GetOptionalOr(optinal.value(), "DepthEnable", true);
		desc.DepthWriteMask = GetDepthWriteMask(GetOptionalOr(optinal.value(), "DepthWriteMask", "All"));
		desc.DepthFunc = GetComparisonFunc(GetOptionalOr(optinal.value(), "DepthFunc", "Less"));
		desc.StencilEnable = GetOptionalOr(optinal.value(), "StencilEnable", false);
		desc.StencilReadMask = GetOptionalOr(optinal.value(), "StencilReadMask", 0);
		desc.StencilWriteMask = GetOptionalOr(optinal.value(), "StencilWriteMask", 0);
		desc.FrontFace = GetDepthStencilOp(optinal->get_child_optional("FrontFace"));
		desc.BackFace = GetDepthStencilOp(optinal->get_child_optional("BackFace"));
	}
	return desc;
}

D3D12_DEPTH_STENCILOP_DESC OPSOReader::GetDepthStencilOp(const boost::optional<const boost::property_tree::ptree&>& Node)
{
	if (!Node)
	{
		return D3D12_DEPTH_STENCILOP_DESC();
	}

	D3D12_DEPTH_STENCILOP_DESC desc;
	desc.StencilFailOp = GetStencilOp(GetOptionalOr(Node, "StencilFailOp", "Keep"));
	desc.StencilDepthFailOp = GetStencilOp(GetOptionalOr(Node, "StencilDepthFailOp", "Keep"));
	desc.StencilPassOp = GetStencilOp(GetOptionalOr(Node, "StencilPassOp", "Keep"));
	desc.StencilFunc = GetComparisonFunc(GetOptionalOr(Node, "StencilFunc", "Always"));
	return desc;
}

D3D12_DEPTH_WRITE_MASK OPSOReader::GetDepthWriteMask(const string& DepthWriteMaskString)
{
	if (DepthWriteMaskString == "Zero")
	{
		return D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	if (DepthWriteMaskString == "All")
	{
		return D3D12_DEPTH_WRITE_MASK_ALL;
	}
	WIN_LOG(Config, Error, "Unknown depth write mask: {}", TEXT(DepthWriteMaskString));
	return D3D12_DEPTH_WRITE_MASK_ZERO;
}

D3D12_COMPARISON_FUNC OPSOReader::GetComparisonFunc(const string& ComparisonFuncString)
{
	if (ComparisonFuncString == "Never")
	{
		return D3D12_COMPARISON_FUNC_NEVER;
	}
	if (ComparisonFuncString == "Less")
	{
		return D3D12_COMPARISON_FUNC_LESS;
	}
	if (ComparisonFuncString == "Equal")
	{
		return D3D12_COMPARISON_FUNC_EQUAL;
	}
	if (ComparisonFuncString == "LessEqual")
	{
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}
	if (ComparisonFuncString == "Greater")
	{
		return D3D12_COMPARISON_FUNC_GREATER;
	}
	if (ComparisonFuncString == "NotEqual")
	{
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	}
	if (ComparisonFuncString == "GreaterEqual")
	{
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	}
	if (ComparisonFuncString == "Always")
	{
		return D3D12_COMPARISON_FUNC_ALWAYS;
	}
	WIN_LOG(Config, Error, "Unknown comparison func: {}", TEXT(ComparisonFuncString));
	return D3D12_COMPARISON_FUNC_NEVER;
}

D3D12_STENCIL_OP OPSOReader::GetStencilOp(const string& StencilOpString)
{
	if (StencilOpString == "Keep")
	{
		return D3D12_STENCIL_OP_KEEP;
	}
	if (StencilOpString == "Zero")
	{
		return D3D12_STENCIL_OP_ZERO;
	}
	if (StencilOpString == "Replace")
	{
		return D3D12_STENCIL_OP_REPLACE;
	}
	if (StencilOpString == "IncrSat")
	{
		return D3D12_STENCIL_OP_INCR_SAT;
	}
	if (StencilOpString == "DecrSat")
	{
		return D3D12_STENCIL_OP_DECR_SAT;
	}
	if (StencilOpString == "Invert")
	{
		return D3D12_STENCIL_OP_INVERT;
	}
	if (StencilOpString == "Incr")
	{
		return D3D12_STENCIL_OP_INCR;
	}
	if (StencilOpString == "Decr")
	{
		return D3D12_STENCIL_OP_DECR;
	}
	WIN_LOG(Config, Error, "Unknown stencil op: {}", TEXT(StencilOpString));
	return D3D12_STENCIL_OP_KEEP;
}

D3D12_COLOR_WRITE_ENABLE OPSOReader::GetColorWriteEnable(const string& ColorWriteEnableString)
{
	if (ColorWriteEnableString == "Red")
	{
		return D3D12_COLOR_WRITE_ENABLE_RED;
	}
	if (ColorWriteEnableString == "Green")
	{
		return D3D12_COLOR_WRITE_ENABLE_GREEN;
	}
	if (ColorWriteEnableString == "Blue")
	{
		return D3D12_COLOR_WRITE_ENABLE_BLUE;
	}
	if (ColorWriteEnableString == "Alpha")
	{
		return D3D12_COLOR_WRITE_ENABLE_ALPHA;
	}
	if (ColorWriteEnableString == "All")
	{
		return D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	WIN_LOG(Config, Error, "Unknown color write enable: {}", TEXT(ColorWriteEnableString));
	return D3D12_COLOR_WRITE_ENABLE_ALL;
}

SShaderArrayText OPSOReader::GetShaderArray(const boost::property_tree::ptree& Node)
{
	SShaderArrayText shaderArray;
	if (auto optional = Node.get_optional<string>("VertexShader"))
	{
		shaderArray.VertexShaderName = optional.value();
	}
	if (auto optional = Node.get_optional<string>("PixelShader"))
	{
		shaderArray.PixelShaderName = optional.value();
	}
	if (auto optional = Node.get_optional<string>("GeometryShader"))
	{
		shaderArray.GeometryShaderName = optional.value();
	}
	if (auto optional = Node.get_optional<string>("HullShader"))
	{
		shaderArray.HullShaderName = optional.value();
	}
	if (auto optional = Node.get_optional<string>("DomainShader"))
	{
		shaderArray.DomainShaderName = optional.value();
	}
	if (auto optional = Node.get_optional<string>("ComputeShader"))
	{
		shaderArray.ComputeShaderName = optional.value();
	}
	return shaderArray;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE OPSOReader::GetTopologyType(const boost::property_tree::ptree& Node)
{
	if (auto optional = Node.get_optional<string>("PrimitiveTopologyType"))
	{
		const auto string = optional.value();
		if (string == "Point")
		{
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		}
		if (string == "Line")
		{
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		}
		if (string == "Triangle")
		{
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
		if (string == "Patch")
		{
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		}
	}
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

void OPSOReader::SetRenderTargetFormats(DXGI_FORMAT* Formats, const boost::property_tree::ptree& Node)
{
	uint32_t counter = 0;

	if (auto node = Node.get_child_optional("RenderTargetFormats"))
	{
		for (auto& format : node.value() | std::views::values)
		{
			Formats[counter] = GetFormat(format.data());
		}
	}
}

DXGI_FORMAT OPSOReader::GetFormat(const boost::property_tree::ptree& Node)
{
	if (auto optional = Node.get_optional<string>("Format"))
	{
		return GetFormat(optional.value());
	}
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

DXGI_FORMAT OPSOReader::GetFormat(const string& FormatString)
{
	if (FormatString == "R8G8B8A8_UNORM")
	{
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	if (FormatString == "R8G8B8A8_UNORM_SRGB")
	{
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
	if (FormatString == "R32G32B32A32_FLOAT")
	{
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	if (FormatString == "R16G16B16A16_FLOAT")
	{
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	if (FormatString == "R16G16B16A16_UNORM")
	{
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	}
	if (FormatString == "R16G16B16A16_UINT")
	{
		return DXGI_FORMAT_R16G16B16A16_UINT;
	}
	if (FormatString == "D24_UNORM_S8_UINT")
	{
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	if (FormatString == "R32_FLOAT")
	{
		return DXGI_FORMAT_R32_FLOAT;
	}
	if (FormatString == "R32_UINT")
	{
		return DXGI_FORMAT_R32_UINT;
	}
	if (FormatString == "R32G32_FLOAT")
	{
		return DXGI_FORMAT_R32G32_FLOAT;
	}
	if (FormatString == "R32G32_UINT")
	{
		return DXGI_FORMAT_R32G32_UINT;
	}
	if (FormatString == "R32G32B32_FLOAT")
	{
		return DXGI_FORMAT_R32G32B32_FLOAT;
	}
	if (FormatString == "R32G32B32_UINT")
	{
		return DXGI_FORMAT_R32G32B32_UINT;
	}
	if (FormatString == "R32G32B32A32_UINT")
	{
		return DXGI_FORMAT_R32G32B32A32_UINT;
	}
	if (FormatString == "R32G32B32A32_FLOAT")
	{
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	if (FormatString == "R8_UNORM")
	{
		return DXGI_FORMAT_R8_UNORM;
	}
	if (FormatString == "R8_UINT")
	{
		return DXGI_FORMAT_R8_UINT;
	}
	if (FormatString == "R8G8_UNORM")
	{
		return DXGI_FORMAT_R8G8_UNORM;
	}
	if (FormatString == "R8G8_UINT")
	{
		return DXGI_FORMAT_R8G8_UINT;
	}
	if (FormatString == "R8G8B8A8_UINT")
	{
		return DXGI_FORMAT_R8G8B8A8_UINT;
	}
	if (FormatString == "R16_UNORM")
	{
		return DXGI_FORMAT_R16_UNORM;
	}
	if (FormatString == "UNKNOWN")
	{
		return DXGI_FORMAT_UNKNOWN;
	}

	LOG(Config, Error, "Unknown format: {}", TEXT(FormatString));
	return DXGI_FORMAT_UNKNOWN;
}

D3D12_PIPELINE_STATE_FLAGS OPSOReader::GetFlags(const boost::property_tree::ptree& Node)
{
	if (auto flag = Node.get_optional<string>("Flags"))
	{
		const auto string = flag.value();
		if (string == "None")
		{
			return D3D12_PIPELINE_STATE_FLAG_NONE;
		}
		if (string == "ToolDebug")
		{
			return D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
		}
	}
	return D3D12_PIPELINE_STATE_FLAG_NONE;
}
