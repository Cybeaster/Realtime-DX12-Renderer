#pragma once
#include "../../../Materials/Material.h"
#include "../../../Utils/EngineHelper.h"
#include "DXHelper.h"
#include "Engine/RenderObject/RenderObject.h"
#include "Timer/Timer.h"
class OGPUWave : public IRenderObject
{
public:
	OGPUWave(ID3D12Device* _Device, ID3D12GraphicsCommandList* _List, int32_t _M, int32_t _N, float dx, float dt, float speed, float damping);
	OGPUWave() = default;
	~OGPUWave() = default;

	UINT GetRowCount() const { return NumRows; }
	UINT GetColumnCount() const { return NumCols; }
	UINT GetVertexCount() const { return VertexCount; }
	UINT GetTriangleCount() const { return TriangleCount; }

	float GetWidth() const { return NumCols * SpatialStep; }
	float GetDepth() const { return NumRows * SpatialStep; }
	float GetSpatialStep() const { return SpatialStep; }

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetDisplacementMapHandle() const { return CurrSolSRVHandle.GPUHandle; }
	auto GetDiplacementMapTexelSize() const
	{
		return DirectX::XMFLOAT2(1.0f / NumCols, 1.0f / NumRows);
	}

	SMaterialParams GetDisplacementParams() const
	{
		SMaterialParams Params;
		Params.Material = FindMaterial(SMaterialNames::Water);
		Params.DisplacementMapTexelSize = GetDiplacementMapTexelSize();
		Params.GridSpatialStep = GetSpatialStep();
		return Params;
	}

	SRenderItemParams GetRIParams()
	{
		SRenderItemParams params;
		params.NumberOfInstances = 1;
		params.bFrustrumCoolingEnabled = false;
		params.Pickable = false;
		params.MaterialParams = GetDisplacementParams();
		return params;
	}

	uint32_t GetNumSRVRequired() const override
	{
		return 6;
	}

	void BuildResources();

	void BuildDescriptors(IDescriptor* Descriptor) override;

	void Update(const STimer& Gt, ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO);
	void Disturb(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, UINT I, UINT J, float Magnitude);

private:
	UINT NumRows;
	UINT NumCols;

	UINT VertexCount;
	UINT TriangleCount;

	// Simulation constants
	float mK[3];

	float TimeStep;
	float SpatialStep;

	ID3D12Device* Device = nullptr;
	ID3D12GraphicsCommandList* CMDList = nullptr;

	SDescriptorPair PrevSolSRVHandle;
	SDescriptorPair CurrSolSRVHandle;
	SDescriptorPair NextSolSRVHandle;

	SDescriptorPair PrevSolUAVHandle;
	SDescriptorPair CurrSolUAVHandle;
	SDescriptorPair NextSolUAVHandle;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> PrevSol = nullptr;
	ComPtr<ID3D12Resource> CurrSol = nullptr;
	ComPtr<ID3D12Resource> NextSol = nullptr;

	ComPtr<ID3D12Resource> PrevUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> CurrUploadBuffer = nullptr;
};
