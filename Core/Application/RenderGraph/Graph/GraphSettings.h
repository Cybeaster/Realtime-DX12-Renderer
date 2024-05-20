#pragma once
#include "Engine/Engine.h"
#include "RenderGraph/Nodes/AABBVisualizer/AabbVisNode.h"
#include "RenderGraph/Nodes/CopyNode/CopyRenderNode.h"
#include "RenderGraph/Nodes/DebugGeometryNode/ODebugGeometryNode.h"
#include "RenderGraph/Nodes/FrustrumDebugNode/FrustumDebugNode.h"
#include "RenderGraph/Nodes/PostProcessNode/PostProcessNode.h"
#include "RenderGraph/Nodes/PresentNode/PresentNode.h"
#include "RenderGraph/Nodes/ReflectionNode/ReflectionNode.h"
#include "RenderGraph/Nodes/RenderNode.h"
#include "RenderGraph/Nodes/SSAO/SsaoNode.h"
#include "RenderGraph/Nodes/ShadowDebugNode/ShadowDebugNode.h"
#include "RenderGraph/Nodes/ShadowNode/ShadowMapNode.h"
#include "RenderGraph/Nodes/TangentNormalDebugNode/TangentNormalDebugNode.h"
#include "RenderGraph/Nodes/UINode/UiRenderNode.h"
#include "Types.h"

using SNodeFactory = function<unique_ptr<ORenderNode>()>;

// clang-format off

static unordered_map<string, SNodeFactory> FactoryMap = {
	{
		{"OpaqueDynamicReflections", []() { return make_unique<OReflectionNode>(); }},
		{"Opaque", []() { return make_unique<ODefaultRenderNode>(); }},
		{"Transparent", []() { return make_unique<ODefaultRenderNode>(); }},
		{"AlphaTested", []() { return make_unique<ODefaultRenderNode>(); }},
		{"PostProcess", []() { return make_unique<OPostProcessNode>(); }},
		{"UI", []() { return make_unique<OUIRenderNode>(); }},
		{"Present", []() { return make_unique<OPresentNode>(); }},
		{"Shadow", []() { return make_unique<OShadowMapNode>(); }},
		{"ShadowDebug", []() { return make_unique<OShadowDebugNode>(); }},
		{"SSAO", []() { return make_unique<OSSAONode>(); }},
		{"CopyTarget", []() { return make_unique<OCopyRenderNode>(OEngine::Get()->GetWindow().lock()); }},
		{"TangentNormalDebug", []() { return make_unique<TangentNormalDebugNode>(); }},
		{"FrustumDebug", []() { return make_unique<OFrustumDebugNode>(); }},
		{"AABBVisualizer", []() { return make_unique<OAABBVisNode>(); }},
		{"DebugBox", []() { return make_unique<ODebugGeometryNode>(); }}
	}
};

// clang-format on
