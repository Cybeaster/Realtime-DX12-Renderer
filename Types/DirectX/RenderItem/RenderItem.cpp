
#include "RenderItem.h"
void ORenderItem::BindResources(ID3D12GraphicsCommandList* CmdList, SFrameResource* Frame) const
{
	auto idxBufferView = Geometry->IndexBufferView();
	auto vertexBufferView = Geometry->VertexBufferView();

	CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CmdList->IASetIndexBuffer(&idxBufferView);
	CmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void ORenderItem::Update(const UpdateEventArgs& Arg) const
{
	for (auto& Component : Components)
	{
		Component->Tick(Arg);
	}
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
SInstanceData* ORenderItem::GetDefaultInstance()
{
	return &Instances[0];
}

const vector<unique_ptr<OComponentBase>>& ORenderItem::GetComponents() const
{
	return Components;
}

void SRenderItemParams::SetPosition(DirectX::XMFLOAT3 P)
{
	Position = P;
}

void SRenderItemParams::SetScale(DirectX::XMFLOAT3 S)
{
	Scale = S;
}