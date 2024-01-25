#pragma once
#include "DXHelper.h"
#include "Timer/Timer.h"
class OGPUWave
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

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetDisplacementMap() const { return CurrSolSrv; }

	static UINT GetDescriptorCount() { return 6; }

	void BuildResources();

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE CpuDescriptor,
	                      CD3DX12_GPU_DESCRIPTOR_HANDLE GpuDescriptor,
	                      UINT DescriptorSize);

	void Update(const STimer& Gt, ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO);
	void Disturb(ID3D12RootSignature* RootSignature, ID3D12PipelineState* PSO, UINT I, UINT J, float Magnitude);

private:
	UINT NumRows;
	UINT NumCols;

	UINT VertexCount;
	UINT TriangleCount;

	//Simulation constants
	float mK[3];

	float TimeStep;
	float SpatialStep;

	ID3D12Device* Device = nullptr;
	ID3D12GraphicsCommandList* CMDList = nullptr;

	CD3DX12_GPU_DESCRIPTOR_HANDLE PrevSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE CurrSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NextSolSrv;

	CD3DX12_GPU_DESCRIPTOR_HANDLE PrevSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE CurrSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NextSolUav;

	// Two for ping-ponging the textures.
	ComPtr<ID3D12Resource> PrevSol = nullptr;
	ComPtr<ID3D12Resource> CurrSol = nullptr;
	ComPtr<ID3D12Resource> NextSol = nullptr;

	ComPtr<ID3D12Resource> PrevUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> CurrUploadBuffer = nullptr;
};
