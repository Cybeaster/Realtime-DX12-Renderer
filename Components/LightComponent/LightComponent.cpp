
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
		DirectX::XMVECTOR scale, rotation, translation;
		DirectX::XMMatrixDecompose(&scale, &rotation, &translation, DirectX::XMLoadFloat4x4(&Owner->GetDefaultInstance()->World));
		Position = Owner->GetDefaultInstance()->World;
		DirectX::XMFLOAT4 pos{};
		DirectX::XMStoreFloat4(&pos, translation);
		GlobalPosition = translation;
		SpotLight.Position = { pos.x, pos.y, pos.z };
		PointLight.Position = { pos.x, pos.y, pos.z };

		MarkDirty();
	}
	UpdateLightData();
}

void OLightComponent::SetDirectionalLight(const SDirectionalLightPayload& Light)
{
	LightType = ELightType::Directional;
	DirectionalLight.Direction = Light.Direction;
	DirectionalLight.Strength = Light.Strength;
}

void OLightComponent::SetPointLight(const SPointLightPayload& Light)
{
	LightType = ELightType::Point;
	PointLight.FallOffStart = Light.FallOffStart;
	PointLight.FallOffEnd = Light.FallOffEnd;
	PointLight.Position = Light.Position;
	PointLight.Strength = Light.Strength;
}

void OLightComponent::SetSpotLight(const SSpotLightPayload& Light)
{
	LightType = ELightType::Spot;
	SpotLight.ConeAngle = Light.ConeAngle;
	SpotLight.Direction = Light.Direction;
	SpotLight.FallOffEnd = Light.FallOffEnd;
	SpotLight.FallOffStart = Light.FallOffStart;
	SpotLight.SpotPower = Light.SpotPower;
	SpotLight.Strength = Light.Strength;
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

	XMFLOAT4X4 transposed{};
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
	XMVECTOR lightDir;
	XMVECTOR lightTarget; // Calculate target point using direction
	XMVECTOR lightPos = GlobalPosition;
	XMMATRIX T(
	    0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);

	if (LightType == ELightType::Directional)
	{
		lightDir = XMVector3Normalize(XMLoadFloat3(&DirectionalLight.Direction));
		lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction

		// Assuming lightDir is already normalized
		auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);

		// Transform bounding sphere to light space.
		XMFLOAT3 sphereCenterLS;
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(lightTarget, view));
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

		XMMATRIX S = view * lightProj * T;
		XMStoreFloat4x4(&View, view);
		XMStoreFloat4x4(&Proj, lightProj);
		XMStoreFloat4x4(&ShadowTransform, S);
	}
	else if (LightType == ELightType::Spot)
	{
		lightDir = XMVector3Normalize(XMLoadFloat3(&SpotLight.Direction));
		lightTarget = XMVectorAdd(lightPos, lightDir); // Calculate target point using direction
		NearZ = SpotLight.FallOffStart;
		FarZ = SpotLight.FallOffEnd;

		// Create the view matrix for the spotlight
		auto view = XMMatrixLookAtLH(lightPos, lightTarget, lightUp);
		auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(SpotLight.ConeAngle * 2), 1.0f, NearZ, FarZ);
		XMMATRIX S = view * projection * T;
		XMStoreFloat4x4(&View, view);
		XMStoreFloat4x4(&Proj, projection);
		XMStoreFloat4x4(&ShadowTransform, S);
	}
	XMStoreFloat3(&LightPos, lightPos);
}

void OLightComponent::SetShadowMapIndex(UINT Index)
{
	DirectionalLight.ShadowMapIndex = Index;
	PointLight.ShadowMapIndex = Index;
	SpotLight.ShadowMapIndex = Index;
}
