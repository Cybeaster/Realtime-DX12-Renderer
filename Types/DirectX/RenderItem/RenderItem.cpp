
#include "RenderItem.h"

#include "FrameResource.h"
void ORenderItem::BindResources(ID3D12GraphicsCommandList* CmdList, SFrameResource* Frame) const
{
	auto idxBufferView = Geometry->IndexBufferView();
	auto vertexBufferView = Geometry->VertexBufferView();

	CmdList->IASetPrimitiveTopology(PrimitiveType);
	CmdList->IASetIndexBuffer(&idxBufferView);
	CmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void ORenderItem::UpdateWorldMatrix(const DirectX::XMMATRIX& WorldMatrix)
{
	XMStoreFloat4x4(&World, WorldMatrix);
	NumFramesDirty = SRenderConstants::NumFrameResources;
}

bool ORenderItem::IsValid() const
{
	return RenderLayer != "NONE" && Geometry != nullptr;
}

bool ORenderItem::IsValidChecked() const
{
	const bool bIsValid = IsValid();
	CWIN_LOG(!bIsValid, Geometry, Error, "RenderLayer is NONE");
	return bIsValid;
}