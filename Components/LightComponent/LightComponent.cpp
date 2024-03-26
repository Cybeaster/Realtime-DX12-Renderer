
#include "LightComponent.h"

#include "Engine/Engine.h"

OLightComponent::OLightComponent(uint32_t DirLightIdx, uint32_t PointLightIdx, uint32_t SpotLightIdx, ELightType Type)
    : DirLightBufferInfo(DirLightIdx), PointLightBufferInfo(PointLightIdx), SpotLightBufferInfo(SpotLightIdx), LightType(Type)
{
	Name = "LightComponent";
}

void OLightComponent::Tick(UpdateEventArgs Arg)
{
	if (!Utils::MatricesEqual(Owner->GetDefaultInstance()->World, Position)) // make delegate based
	{
		Position = Owner->GetDefaultInstance()->World;
		auto matrix = DirectX::XMLoadFloat4x4(&Position);
		DirectX::XMFLOAT4 pos = { matrix.r[3].m128_f32[0], matrix.r[3].m128_f32[1], matrix.r[3].m128_f32[2], matrix.r[3].m128_f32[3] };
		GlobalPosition = DirectX::XMLoadFloat4(&pos);
		SpotLight.Position = { pos.x, pos.y, pos.z };
		PointLight.Position = { pos.x, pos.y, pos.z };

		MarkDirty();
	}

	UpdateLightData();
}

void OLightComponent::SetDirectionalLight(const SDirectionalLight& Light)
{
	LightType = ELightType::Directional;
	DirectionalLight = Light;
}

void OLightComponent::SetPointLight(const SPointLight& Light)
{
	LightType = ELightType::Point;
	PointLight = Light;
}

void OLightComponent::SetSpotLight(const SSpotLight& Light)
{
	LightType = ELightType::Spot;
	SpotLight = Light;
}

void OLightComponent::Init(ORenderItem* Other)
{
	OSceneComponent::Init(Other);
}

void OLightComponent::UpdateFrameResource(const SFrameResource* FrameResource)
{
	switch (LightType)
	{
	case ELightType::Directional:
		FrameResource->DirectionalLightBuffer->CopyData(DirLightBufferInfo.StartIndex, DirectionalLight);
		break;
	case ELightType::Point:
		FrameResource->PointLightBuffer->CopyData(PointLightBufferInfo.StartIndex, PointLight);
		break;
	case ELightType::Spot:
		FrameResource->SpotLightBuffer->CopyData(SpotLightBufferInfo.StartIndex, SpotLight);
		break;
	}
}

void OLightComponent::InitFrameResource(const TUploadBufferData<SDirectionalLight>& Dir, const TUploadBufferData<SPointLight>& Point, const TUploadBufferData<SSpotLight>& Spot)
{
	DirLightBufferInfo = Dir;
	PointLightBufferInfo = Point;
	SpotLightBufferInfo = Spot;
}

bool OLightComponent::TryUpdate()
{
	bool res = NumFramesDirty > 0;
	NumFramesDirty = 0;
	return res;
}

ELightType OLightComponent::GetLightType() const
{
	return LightType;
}

int32_t OLightComponent::GetLightIndex() const
{
	switch (LightType)
	{
	case ELightType::Directional:
		return DirLightBufferInfo.StartIndex;
		break;
	case ELightType::Point:
		return PointLightBufferInfo.StartIndex;
		break;
	case ELightType::Spot:
		return SpotLightBufferInfo.StartIndex;
		break;
	}
	LOG(Render, Error, "Light type not initized")
	return -1;
}

SDirectionalLight& OLightComponent::GetDirectionalLight()
{
	return DirectionalLight;
}

SPointLight& OLightComponent::GetPointLight()
{
	return PointLight;
}

SSpotLight& OLightComponent::GetSpotLight()
{
	return SpotLight;
}

void OLightComponent::MarkDirty()
{
	LOG(Render, Log, "Marking light as dirty");
	NumFramesDirty = SRenderConstants::NumFrameResources;
}

void OLightComponent::SetPassConstant(SPassConstants& OutConstant)
{
	using namespace DirectX;

	const auto view = XMLoadFloat4x4(&View);
	const auto proj = XMLoadFloat4x4(&Proj);
	const auto shadowTransform = XMLoadFloat4x4(&ShadowTransform);

	XMFLOAT4X4 transposed;
	Put(transposed, Transpose(shadowTransform));
	DirectionalLight.Transform = transposed;
	PointLight.Transform = transposed;
	SpotLight.Transform = transposed;

	const XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	const XMMATRIX invView = Inverse(view);
	const XMMATRIX invProj = Inverse(proj);
	const XMMATRIX invViewProj = Inverse(viewProj);

	XMStoreFloat4x4(&OutConstant.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&OutConstant.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&OutConstant.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&OutConstant.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&OutConstant.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&OutConstant.InvViewProj, XMMatrixTranspose(invViewProj));

	OutConstant.EyePosW = LightPos;
	OutConstant.NearZ = NearZ;
	OutConstant.FarZ = FarZ;
}

void OLightComponent::UpdateLightData()
{
	using namespace DirectX;

	auto lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const auto& bounds = OEngine::Get()->GetSceneBounds();
	XMFLOAT3 dir = { 0.57735f, -0.57735f, 0.57735f };
	XMVECTOR lightDir = XMLoadFloat3(&dir);
	XMVECTOR lightPos = GlobalPosition;
	XMVECTOR lightTarget; // Calculate target point using direction
	switch (LightType)
	{
	case ELightType::Directional:
		lightDir = XMLoadFloat3(&DirectionalLight.Direction);
		lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction
		break;
	case ELightType::Point:
		lightDir = XMLoadFloat3(&bounds.Center);
		lightTarget = lightDir; // Calculate target point using direction
		break;
	case ELightType::Spot:
		lightDir = XMLoadFloat3(&SpotLight.Direction);
		lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction

		break;
	}
	// Assuming lightDir is already normalized
	auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(lightTarget, view));
	XMStoreFloat3(&LightPos, lightPos);
	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - bounds.Radius;
	float b = sphereCenterLS.y - bounds.Radius;
	float n = sphereCenterLS.z - bounds.Radius;
	float r = sphereCenterLS.x + bounds.Radius;
	float t = sphereCenterLS.y + bounds.Radius;
	float f = sphereCenterLS.z + bounds.Radius;

	NearZ = n;
	FarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
	    0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = view * lightProj * T;
	XMStoreFloat4x4(&View, view);
	XMStoreFloat4x4(&Proj, lightProj);
	XMStoreFloat4x4(&ShadowTransform, S);
}

void OLightComponent::SetShadowMapIndex(UINT Index)
{
	switch (LightType)
	{
	case ELightType::Directional:
		DirectionalLight.ShadowMapIndex = Index;
		break;
	case ELightType::Point:
		PointLight.ShadowMapIndex = Index; // doesn't make sense for now
		break;
	case ELightType::Spot:
		SpotLight.ShadowMapIndex = Index;
		break;
	}
}
